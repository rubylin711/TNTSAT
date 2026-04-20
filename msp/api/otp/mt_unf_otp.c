/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "mt_type.h"
#include "mt_unf_otp.h"
#include "mt_module_debug.h"

/*****************************************OTP MAP********************************************/

/*******SYM 1/3*******/
#define SYMPHONY_OTP_SRC_MAX_BLOCK_NUM   62

#define OTP_MAP_SYMPHONY_CHIP_ID            (0xfc0)
#define OTP_MAP_SYMPHONY_CHIP_ID_CA         (0xf80)

#define OTP_MAP_SYMPHONY_DOLBY_ENABLE       (0xebe)
#define OTP_MAP_SYMPHONY_DOLBY_ENABLE_SIZE  (2)

#define OTP_MAP_SYMPHONY_DRA_ENABLE         (0xebc)
#define OTP_MAP_SYMPHONY_DRA_ENABLE_SIZE    (2)

#define OTP_MAP_SYMPHONY_HD_DEC_ENABLE      (0xeba)
#define OTP_MAP_SYMPHONY_HD_DEC_ENABLE_SIZE (2)

#define OTP_MAP_SYMPHONY_EPHY_ENABLE         (0xeb8)
#define OTP_MAP_SYMPHONY_EPHY_ENABLE_SIZE    (2)

#define OTP_MAP_SYMPHONY_H265_ENABLE        (0x915)
#define OTP_MAP_SYMPHONY_H265_ENABLE_SIZE   (1)

#define OTP_MAP_SYMPHONY_CW_CFG_ENABLE      (0xefa*32+15)
#define OTP_MAP_SYMPHONY_CW_CFG_ENABLE_SIZE (1)

#define OTP_MAP_SYMPHONY_HD_ENABLE          (0x8cf)
#define OTP_MAP_SYMPHONY_HD_ENABLE_SIZE     (1)

#define OTP_MAP_SYMPHONY_PACKET_TYPE        (0xeb0)
#define OTP_MAP_SYMPHONY_PACKET_TYPE_SIZE   (8)

#define OTP_MAP_SYMPHONY_DOLBY_FORCE_DISABLE       (0xea2)
#define OTP_MAP_SYMPHONY_DOLBY_FORCE_DISABLE_SIZE  (1)

#define OTP_MAP_SYMPHONY_CTRL_AREA_0        (0x800)
#define OTP_MAP_SYMPHONY_CTRL_AREA_SIZE     (2)

/*******SYM 2*******/
#define OTP_MAP_SYMPHONY2_CHIP_ID           (0xf06*32)

#define OTP_MAP_SYMPHONY2_CHIP_ID_CA        (0xfda*32)
#define OTP_MAP_SYMPHONY2_CASID		    	(0xeff*32)

#define OTP_MAP_SYMPHONY2_DOLBY_ENABLE       (0xea3*32+11)
#define OTP_MAP_SYMPHONY2_DOLBY_ENABLE_SIZE  (7)

#define OTP_MAP_SYMPHONY2_DRA_ENABLE         (0xea3*32+6)
#define OTP_MAP_SYMPHONY2_DRA_ENABLE_SIZE    (1)

#define OTP_MAP_SYMPHONY2_HD_DEC_ENABLE      (0xea3*32+7)
#define OTP_MAP_SYMPHONY2_HD_DEC_ENABLE_SIZE (2)

#define OTP_MAP_SYMPHONY2_PACKET_TYPE        (0xea3*32 + 28)
#define OTP_MAP_SYMPHONY2_PACKET_TYPE_SIZE   (4)

#define OTP_MAP_SYMPHONY2_DRAM_CAP			(0xea3 *32 + 4)
#define OTP_MAP_SYMPHONY2_DRAM_CAP_SIZE		(4)

#define OTP_MAP_SYMPHONY2_PRODUCT_TYPE			(0xea2*32 + 28)
#define OTP_MAP_SYMPHONY2_CHIP_FAMILY			(0xea2*32 + 26)
#define OTP_MAP_SYMPHONY2_CHIP_GENERATION		(0xea2*32 + 24)
#define OTP_MAP_SYMPHONY2_DISPLAY_RESOLUTION		(0xea2*32 + 20)
#define OTP_MAP_SYMPHONY2_SECURITY_TYPE			(0xea2*32 + 18)
#define OTP_MAP_SYMPHONY2_PACKAGE_INFO			(0xea3*32 + 28)
#define OTP_MAP_SYMPHONY2_SIP_DRAM_SIZE			(0xea3*32 + 24)
#define OTP_MAP_SYMPHONY2_CA_VENDOR			(0xea3*32 + 20)
#define OTP_MAP_SYMPHONY2_CA_VERSION			(0xea3*32 + 18)
#define OTP_MAP_SYMPHONY2_DRM_INFO			(0xea3*32 + 12)
#define OTP_MAP_SYMPHONY2_IP_LICENSE			(0xea3*32 +  8)
#define OTP_MAP_SYMPHONY2_DDR_KGD			(0xea3*32 +  0)
#define OTP_MAP_SYMPHONY2_BLW0_ADDR			(0xef0*32 + 14)
#define OTP_MAP_SYMPHONY2_BLW0_DATA			(3)
#define OTP_MAP_SYMPHONY2_SWL_BLW0_ADDR			(0xf97*32 + 23)
#define OTP_MAP_SYMPHONY2_SWL_BLW0_DATA			(7)

#define OTP_MAP_SYMPHONY2_PRODUCT_TYPE_SIZE		(4)
#define OTP_MAP_SYMPHONY2_CHIP_FAMILY_SIZE		(2)
#define OTP_MAP_SYMPHONY2_CHIP_GENERATION_SIZE		(2)
#define OTP_MAP_SYMPHONY2_DISPLAY_RESOLUTION_SIZE	(4)
#define OTP_MAP_SYMPHONY2_SECURITY_TYPE_SIZE		(2)
#define OTP_MAP_SYMPHONY2_PACKAGE_INFO_SIZE		(4)
#define OTP_MAP_SYMPHONY2_SIP_DRAM_SIZE_SIZE		(4)
#define OTP_MAP_SYMPHONY2_CA_VENDOR_SIZE		(4)
#define OTP_MAP_SYMPHONY2_CA_VERSION_SIZE		(2)
#define OTP_MAP_SYMPHONY2_DRM_INFO_SIZE			(4)
#define OTP_MAP_SYMPHONY2_IP_LICENSE_SIZE		(4)
#define OTP_MAP_SYMPHONY2_DDR_KGD_SIZE			(4)

#define OTP_MAP_SYMPHONY2_HD_ENABLE				(0xf04*32 + 31)
#define OTP_MAP_SYMPHONY2_MACROVISION_ENABLE	(0xf04*32 + 30)
#define OTP_MAP_SYMPHONY2_H265_ENABLE			(0xf04*32 + 29)
#define OTP_MAP_SYMPHONY2_USB0_ENABLE			(0xf04*32 + 28)
#define OTP_MAP_SYMPHONY2_USB1_ENABLE			(0xf04*32 + 27)
#define OTP_MAP_SYMPHONY2_DVBT2_ENABLE			(0xf04*32 + 26)
#define OTP_MAP_SYMPHONY2_DVBT_ENABLE			(0xf04*32 + 25)
#define OTP_MAP_SYMPHONY2_DVBS2X_ENABLE			(0xf04*32 + 24)
#define OTP_MAP_SYMPHONY2_DVBS_S2_S2X_ENABLE	(0xf04*32 + 23)
#define OTP_MAP_SYMPHONY2_J83B_ENABLE			(0xf04*32 + 22)
#define OTP_MAP_SYMPHONY2_DVBC_ENABLE			(0xf04*32 + 21)
#define OTP_MAP_SYMPHONY2_EPHY_ENABLE			(0xf04*32 + 20)
#define OTP_MAP_SYMPHONY2_HDR_ENABLE			(0xf04*32 + 19)
#define OTP_MAP_SYMPHONY2_VDEC10B_ENABLE		(0xf04*32 + 18)
#define OTP_MAP_SYMPHONY2_OTP_SMCD_ENABLE		(0xf04*32 + 17)
#define OTP_MAP_SYMPHONY2_OTP_SPIMEMMAP_ENABLE	(0xf04*32 + 16)
#define OTP_MAP_SYMPHONY2_OTP_ETHD_ENABLE		(0xf04*32 + 14)

#define OTP_MAP_SYMPHONY2_HD_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY2_MACROVISION_ENABLE_SIZE	(1)
#define OTP_MAP_SYMPHONY2_H265_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY2_USB0_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY2_USB1_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY2_DVBT2_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY2_DVBT_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY2_DVBS2X_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY2_DVBS_S2_S2X_ENABLE_SIZE	(1)
#define OTP_MAP_SYMPHONY2_J83B_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY2_DVBC_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY2_EPHY_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY2_HDR_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY2_VDEC10B_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY2_OTP_SMCD_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY2_OTP_SPIMEMMAP_ENABLE_SIZE	(1)
#define OTP_MAP_SYMPHONY2_OTP_ETHD_ENABLE_SIZE		(2)
#define OTP_MAP_SYMPHONY2_BLW0_SIZE			(2)
#define OTP_MAP_SYMPHONY2_SWL_BLW0_SIZE			(3)

/*******SYM 4*******/
#define OTP_MAP_SYMPHONY4_CHIP_ID           	(0xff2*32)
#define OTP_MAP_SYMPHONY4_CHIP_ID_CA        	(0xfab*32)
#define OTP_MAP_SYMPHONY4_PRODUCT_TYPE			(0xff0*32 + 28)
#define OTP_MAP_SYMPHONY4_CHIP_FAMILY			(0xff0*32 + 26)
#define OTP_MAP_SYMPHONY4_CHIP_GENERATION		(0xff0*32 + 24)
#define OTP_MAP_SYMPHONY4_DISPLAY_RESOLUTION	(0xff0*32 + 20)
#define OTP_MAP_SYMPHONY4_SECURITY_TYPE			(0xff0*32 + 16)

#define OTP_MAP_SYMPHONY4_PRODUCT_TYPE_SIZE		(4)
#define OTP_MAP_SYMPHONY4_CHIP_FAMILY_SIZE		(2)
#define OTP_MAP_SYMPHONY4_CHIP_GENERATION_SIZE		(2)
#define OTP_MAP_SYMPHONY4_DISPLAY_RESOLUTION_SIZE	(4)
#define OTP_MAP_SYMPHONY4_SECURITY_TYPE_SIZE		(4)

#define OTP_MAP_SYMPHONY4_PACKAGE_INFO		(0xff1*32 + 28)
#define OTP_MAP_SYMPHONY4_SIP_DRAM_SIZE		(0xff1*32 + 24)
#define OTP_MAP_SYMPHONY4_CA_VENDOR			(0xff1*32 + 18)
#define OTP_MAP_SYMPHONY4_CA_VERSION		(0xff1*32 + 14)
#define OTP_MAP_SYMPHONY4_DRM_INFO			(0xff1*32 + 10)
#define OTP_MAP_SYMPHONY4_IP_LICENSE		(0xff1*32 +  6)
#define OTP_MAP_SYMPHONY4_DDR_KGD			(0xff1*32 +  0)
#define OTP_MAP_SYMPHONY4_BLW0_ADDR			(0xe99*32 + 14)
#define OTP_MAP_SYMPHONY4_SWL_BLW0_ADDR		(0xf5c*32 + 23)
#define OTP_MAP_SYMPHONY4_BLW0_DATA			(3)

#define OTP_MAP_SYMPHONY4_PACKAGE_INFO_SIZE		(4)
#define OTP_MAP_SYMPHONY4_SIP_DRAM_SIZE_SIZE	(4)
#define OTP_MAP_SYMPHONY4_CA_VENDOR_SIZE		(6)
#define OTP_MAP_SYMPHONY4_CA_VERSION_SIZE		(4)
#define OTP_MAP_SYMPHONY4_DRM_INFO_SIZE			(4)
#define OTP_MAP_SYMPHONY4_IP_LICENSE_SIZE		(4)
#define OTP_MAP_SYMPHONY4_DDR_KGD_SIZE			(4)
#define OTP_MAP_SYMPHONY4_BLW0_SIZE				(2)
#define OTP_MAP_SYMPHONY4_SWL_BLW0_SIZE			(3)

#define OTP_MAP_SYMPHONY4_HD_ENABLE				(0xea8*32 + 31)
#define OTP_MAP_SYMPHONY4_MACROVISION_ENABLE	(0xea8*32 + 30)
#define OTP_MAP_SYMPHONY4_H265_ENABLE			(0xea8*32 + 29)
#define OTP_MAP_SYMPHONY4_USB0_ENABLE			(0xea8*32 + 28)
#define OTP_MAP_SYMPHONY4_USB1_ENABLE			(0xea8*32 + 27)
#define OTP_MAP_SYMPHONY4_J83B_ENABLE			(0xea8*32 + 26)
#define OTP_MAP_SYMPHONY4_DVBC_ENABLE			(0xea8*32 + 25)
#define OTP_MAP_SYMPHONY4_DVBS2X_ENABLE			(0xea8*32 + 22)
#define OTP_MAP_SYMPHONY4_DVBS_ENABLE			(0xea8*32 + 21)
#define OTP_MAP_SYMPHONY4_EPHY_ENABLE			(0xea8*32 + 20)
#define OTP_MAP_SYMPHONY4_HDR_ENABLE			(0xea8*32 + 19)
#define OTP_MAP_SYMPHONY4_VDEC10B_ENABLE		(0xea8*32 + 18)
#define OTP_MAP_SYMPHONY4_OTP_SMCD_ENABLE		(0xea8*32 + 17)
#define OTP_MAP_SYMPHONY4_OTP_HDRX_ENABLE		(0xea8*32 + 16)
#define OTP_MAP_SYMPHONY4_OTP_ETHD_ENABLE		(0xea8*32 + 15)
#define OTP_MAP_SYMPHONY4_OTP_NGWM_ENABLE		(0xea8*32 + 13)
#define OTP_MAP_SYMPHONY4_OTP_VMXWM_ENABLE		(0xea8*32 + 11)
#define OTP_MAP_SYMPHONY4_OTP_VP9_ENABLE		(0xea8*32 + 10)
#define OTP_MAP_SYMPHONY4_OTP_CORE1_ENABLE		(0xea8*32 + 8)
#define OTP_MAP_SYMPHONY4_OTP_CPC_ENABLE		(0xea8*32 + 7)
#define OTP_MAP_SYMPHONY4_OTP_LCD_ENABLE		(0xea8*32)

#define OTP_MAP_SYMPHONY4_HD_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY4_MACROVISION_ENABLE_SIZE	(1)
#define OTP_MAP_SYMPHONY4_H265_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY4_USB0_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY4_USB1_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY4_J83B_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY4_DVBC_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY4_DVBS2X_ENABLE_SIZE	(1)
#define OTP_MAP_SYMPHONY4_DVBS_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY4_EPHY_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY4_HDR_ENABLE_SIZE		(1)
#define OTP_MAP_SYMPHONY4_VDEC10B_ENABLE_SIZE	(1)
#define OTP_MAP_SYMPHONY4_OTP_SMCD_ENABLE_SIZE	(1)
#define OTP_MAP_SYMPHONY4_OTP_HDRX_ENABLE_SIZE	(1)
#define OTP_MAP_SYMPHONY4_OTP_ETHD_ENABLE_SIZE	(1)
#define OTP_MAP_SYMPHONY4_OTP_NGWM_ENABLE_SIZE	(2)
#define OTP_MAP_SYMPHONY4_OTP_VMXWM_ENABLE_SIZE	(2)
#define OTP_MAP_SYMPHONY4_OTP_VP9_ENABLE_SIZE	(1)
#define OTP_MAP_SYMPHONY4_OTP_CORE1_ENABLE_SIZE	(1)
#define OTP_MAP_SYMPHONY4_OTP_CPC_ENABLE_SIZE	(1)
#define OTP_MAP_SYMPHONY4_OTP_LCD_ENABLE_SIZE	(1)

#define OTP_MAP_SYMPHONY4_DOLBY_FORCE_DISABLE       (0xfb5*32 + 30)
#define OTP_MAP_SYMPHONY4_DOLBY_FORCE_DISABLE_SIZE  (2)

#define OTP_MAP_SYMPHONY4_DRA_ENABLE       			(0xfb5*32 + 29)
#define OTP_MAP_SYMPHONY4_DRA_ENABLE_SIZE  			(1)

/*******SYM 6*******/
/* 0xFF2/0x3FC8: Chip Production Lot Number (64bit) */
#define OTP_MAP_SYMPHONY6_CHIP_ID           		(0xff2*32)

/* 0xFAB/0x3EAC: CAS Chip ID (64bit) */
#define OTP_MAP_SYMPHONY6_CAS_CHIP_ID           	(0xfab*32)

/* 0xFF0/0x3FC0 */
#define OTP_MAP_SYMPHONY6_SUBFEATURE				(0xff0*32 + 15)
#define OTP_MAP_SYMPHONY6_SUBFEATURE_SIZE			(5)

#define OTP_MAP_SYMPHONY6_DISPLAY_RESOLUTION		(0xff0*32 + 20)
#define OTP_MAP_SYMPHONY6_DISPLAY_RESOLUTION_SIZE	(4)

#define OTP_MAP_SYMPHONY6_CHIP_GENERATION			(0xff0*32 + 24)
#define OTP_MAP_SYMPHONY6_CHIP_GENERATION_SIZE		(2)

#define OTP_MAP_SYMPHONY6_CHIP_FAMILY				(0xff0*32 + 26)
#define OTP_MAP_SYMPHONY6_CHIP_FAMILY_SIZE			(2)

#define OTP_MAP_SYMPHONY6_PRODUCT_TYPE				(0xff0*32 + 28)
#define OTP_MAP_SYMPHONY6_PRODUCT_TYPE_SIZE			(4)

/* 0xFF1/0x3FC4 */
#define OTP_MAP_SYMPHONY6_IP_LICENSE				(0xff1*32 +  6)
#define OTP_MAP_SYMPHONY6_IP_LICENSE_SIZE			(4)

#define OTP_MAP_SYMPHONY6_HWIP_INFO					(0xff1*32 + 10)
#define OTP_MAP_SYMPHONY6_HWIP_INFO_SIZE			(4)

#define OTP_MAP_SYMPHONY6_CHIPSET_VERSION			(0xff1*32 + 14)
#define OTP_MAP_SYMPHONY6_CHIPSET_VERSION_SIZE		(4)

/* A0 */
#define OTP_MAP_SYMPHONY6_CA_VENDOR					(0xff1*32 + 18)
#define OTP_MAP_SYMPHONY6_CA_VENDOR_SIZE			(5)
/* A1 */
#define OTP_MAP_SYMPHONY6_A1_CA_VENDOR				(0xff1*32 + 18)
#define OTP_MAP_SYMPHONY6_A1_CA_VENDOR_SIZE			(6)

/* A0 */
#define OTP_MAP_SYMPHONY6_SIP_DRAM_SIZE				(0xff1*32 + 23)
#define OTP_MAP_SYMPHONY6_SIP_DRAM_SIZE_SIZE		(5)
/* A1 */
#define OTP_MAP_SYMPHONY6_A1_SIP_DRAM_SIZE			(0xff1*32 + 24)
#define OTP_MAP_SYMPHONY6_A1_SIP_DRAM_SIZE_SIZE		(4)

#define OTP_MAP_SYMPHONY6_PACKAGE_INFO				(0xff1*32 + 28)
#define OTP_MAP_SYMPHONY6_PACKAGE_INFO_SIZE			(4)

/* 0xFB5/0x3ED4 */
#define OTP_MAP_SYMPHONY6_DRA_ENABLE       			(0xfb5*32 + 29)
#define OTP_MAP_SYMPHONY6_DRA_ENABLE_SIZE  			(1)

/* A0 */
#define OTP_MAP_SYMPHONY6_DOLBY_FORCE_DISABLE       (0xfb5*32 + 30)
#define OTP_MAP_SYMPHONY6_DOLBY_FORCE_DISABLE_SIZE  (2)
/* A1 */
#define OTP_MAP_SYMPHONY6_A1_DOLBY_FORCE_DISABLE       (0xfb5*32 + 31)
#define OTP_MAP_SYMPHONY6_A1_DOLBY_FORCE_DISABLE_SIZE  (1)

/* 0xEA8/0x3AA0 */
#define OTP_MAP_SYMPHONY6_RV_DIS     				(0xea8*32 + 2)
#define OTP_MAP_SYMPHONY6_RV_DIS_SIZE  				(1)

#define OTP_MAP_SYMPHONY6_HDR10P_DIS     			(0xea8*32 + 3)
#define OTP_MAP_SYMPHONY6_HDR10P_DIS_SIZE  			(1)

#define OTP_MAP_SYMPHONY6_HDMI_EMP_DIS     			(0xea8*32 + 5)
#define OTP_MAP_SYMPHONY6_HDMI_EMP_DIS_SIZE  		(1)

#define OTP_MAP_SYMPHONY6_HDMI_DIS     				(0xea8*32 + 6)
#define OTP_MAP_SYMPHONY6_HDMI_DIS_SIZE  			(1)

#define OTP_MAP_SYMPHONY6_CPC_DIS     				(0xea8*32 + 7)
#define OTP_MAP_SYMPHONY6_CPC_DIS_SIZE  			(1)

#define OTP_MAP_SYMPHONY6_ARM_CORE1_DIS  			(0xea8*32 + 8)
#define OTP_MAP_SYMPHONY6_ARM_CORE1_DIS_SIZE  		(1)

#define OTP_MAP_SYMPHONY6_G31_DIS  					(0xea8*32 + 9)
#define OTP_MAP_SYMPHONY6_G31_DIS_SIZE  			(1)

#define OTP_MAP_SYMPHONY6_VP9_DIS  					(0xea8*32 + 10)
#define OTP_MAP_SYMPHONY6_VP9_DIS_SIZE  			(1)

#define OTP_MAP_SYMPHONY6_USB1_30_DIS  				(0xea8*32 + 14)
#define OTP_MAP_SYMPHONY6_USB1_30_DIS_SIZE  		(1)

#define OTP_MAP_SYMPHONY6_EMAC_DIS  				(0xea8*32 + 15)
#define OTP_MAP_SYMPHONY6_EMAC_DIS_SIZE  			(1)

#define OTP_MAP_SYMPHONY6_SL_HDR1_DIS  				(0xea8*32 + 16)
#define OTP_MAP_SYMPHONY6_SL_HDR1_DIS_SIZE  		(1)

#define OTP_MAP_SYMPHONY6_SMC_DIS  					(0xea8*32 + 17)
#define OTP_MAP_SYMPHONY6_SMC_DIS_SIZE  			(1)

#define OTP_MAP_SYMPHONY6_VDEC10B_DIS  				(0xea8*32 + 18)
#define OTP_MAP_SYMPHONY6_VDEC10B_DIS_SIZE  		(1)

#define OTP_MAP_SYMPHONY6_HDR10_HLG_DIS  			(0xea8*32 + 19)
#define OTP_MAP_SYMPHONY6_HDR10_HLG_DIS_SIZE  		(1)

#define OTP_MAP_SYMPHONY6_GMAC_DIS  				(0xea8*32 + 20)
#define OTP_MAP_SYMPHONY6_GMAC_DIS_SIZE  			(1)

#define OTP_MAP_SYMPHONY6_DVBS_S2_DIS  				(0xea8*32 + 21)
#define OTP_MAP_SYMPHONY6_DVBS_S2_DIS_SIZE  		(1)

#define OTP_MAP_SYMPHONY6_DVBS2X_DIS  				(0xea8*32 + 22)
#define OTP_MAP_SYMPHONY6_DVBS2X_DIS_SIZE 	 		(1)

#define OTP_MAP_SYMPHONY6_DVBT_DIS  				(0xea8*32 + 23)
#define OTP_MAP_SYMPHONY6_DVBT_DIS_SIZE 	 		(1)

#define OTP_MAP_SYMPHONY6_DVBC_DIS  				(0xea8*32 + 25)
#define OTP_MAP_SYMPHONY6_DVBC_DIS_SIZE 	 		(1)

#define OTP_MAP_SYMPHONY6_J83B_DIS  				(0xea8*32 + 26)
#define OTP_MAP_SYMPHONY6_J83B_DIS_SIZE 	 		(1)

#define OTP_MAP_SYMPHONY6_USB1_DIS  				(0xea8*32 + 27)
#define OTP_MAP_SYMPHONY6_USB1_DIS_SIZE 	 		(1)

#define OTP_MAP_SYMPHONY6_USB0_DIS  				(0xea8*32 + 28)
#define OTP_MAP_SYMPHONY6_USB0_DIS_SIZE 	 		(1)

#define OTP_MAP_SYMPHONY6_H265_DIS  				(0xea8*32 + 29)
#define OTP_MAP_SYMPHONY6_H265_DIS_SIZE 	 		(1)

#define OTP_MAP_SYMPHONY6_MACROVISION_DIS  			(0xea8*32 + 30)
#define OTP_MAP_SYMPHONY6_MACROVISION_DIS_SIZE 	 	(1)

#define OTP_MAP_SYMPHONY6_UHD_DIS  					(0xea8*32 + 31)
#define OTP_MAP_SYMPHONY6_UHD_DIS_SIZE 	 			(1)

/********************************************************************************************/

static int OtpHandle 		= -1;
static MT_CHIP_VERSION_E otp_version 		= MT_CHIP_VERSION_BUTT;
MT_U32 otp_casid		= (MT_U32)-1;
MT_U32 max_bit_addr		= (MT_U32)-1;
MT_U32 chip_id_ca		= (MT_U32)-1;
MT_U32 ctrl_area_0		= (MT_U32)-1;
MT_U32 ctrl_area_size		= (MT_U32)-1;
MT_U32 src_max_block_num	= (MT_U32)-1;
MT_U32 packet_type		= (MT_U32)-1;
MT_U32 packet_type_size		= (MT_U32)-1;
MT_U32 chip_id			= (MT_U32)-1;
MT_U32 hd_enable		= (MT_U32)-1;
MT_U32 hd_enable_size		= (MT_U32)-1;
MT_U32 hd_dec_enable		= (MT_U32)-1;
MT_U32 hd_dec_enable_size	= (MT_U32)-1;
MT_U32 dolby_enable		= (MT_U32)-1;
MT_U32 dolby_enable_size	= (MT_U32)-1;
MT_U32 dra_enable		= (MT_U32)-1;
MT_U32 dra_enable_size		= (MT_U32)-1;
MT_U32 h265_enable		= (MT_U32)-1;
MT_U32 h265_enable_size		= (MT_U32)-1;
MT_U32 cw_cfg_enable		= (MT_U32)-1;
MT_U32 cw_cfg_enable_size	= (MT_U32)-1;
MT_U32 product			= (MT_U32)-1;
MT_U32 product_size		= (MT_U32)-1;
MT_U32 family			= (MT_U32)-1;
MT_U32 family_size		= (MT_U32)-1;
MT_U32 generation		= (MT_U32)-1;
MT_U32 generation_size		= (MT_U32)-1;
MT_U32 dram_cap			= (MT_U32)-1;
MT_U32 dram_cap_size		= (MT_U32)-1;
MT_U32 ddr_kgd			= (MT_U32)-1;
MT_U32 ddr_kgd_size		= (MT_U32)-1;

MT_U32 display_resolution	= (MT_U32)-1;
MT_U32 security			= (MT_U32)-1;
MT_U32 package_info		= (MT_U32)-1;
MT_U32 sipdramsize		= (MT_U32)-1;
MT_U32 ca_vendor		= (MT_U32)-1;
MT_U32 ca_version		= (MT_U32)-1;
MT_U32 drm_info			= (MT_U32)-1;
MT_U32 ip_license		= (MT_U32)-1;

MT_U32 display_resolution_size	= (MT_U32)-1;
MT_U32 security_size		= (MT_U32)-1;
MT_U32 package_info_size	= (MT_U32)-1;
MT_U32 sipdramsize_size		= (MT_U32)-1;
MT_U32 ca_vendor_size		= (MT_U32)-1;
MT_U32 ca_version_size		= (MT_U32)-1;
MT_U32 drm_info_size		= (MT_U32)-1;
MT_U32 ip_license_size		= (MT_U32)-1;


MT_U32 macrovision_enable		= (MT_U32)-1;
MT_U32 usb0_enable			= (MT_U32)-1;
MT_U32 usb1_enable			= (MT_U32)-1;
MT_U32 dvbt2_enable			= (MT_U32)-1;
MT_U32 dvbt_enable			= (MT_U32)-1;
MT_U32 dvbs2x_enable			= (MT_U32)-1;
MT_U32 dvbs_s2_s2x_enable		= (MT_U32)-1;
MT_U32 j83b_enable			= (MT_U32)-1;
MT_U32 dvbc_enable			= (MT_U32)-1;
MT_U32 ephy_enable			= (MT_U32)-1;
MT_U32 hdr_enable			= (MT_U32)-1;
MT_U32 vdec10b_enable			= (MT_U32)-1;
MT_U32 otp_smcd_enable			= (MT_U32)-1;
MT_U32 otp_spimemmap_enable		= (MT_U32)-1;
MT_U32 otp_ethd_enable			= (MT_U32)-1;

MT_U32 macrovision_enable_size 		= (MT_U32)-1;
MT_U32 usb0_enable_size 		= (MT_U32)-1;
MT_U32 usb1_enable_size 		= (MT_U32)-1;
MT_U32 dvbt2_enable_size 		= (MT_U32)-1;
MT_U32 dvbt_enable_size 		= (MT_U32)-1;
MT_U32 dvbs2x_enable_size 		= (MT_U32)-1;
MT_U32 dvbs_s2_s2x_enable_size 		= (MT_U32)-1;
MT_U32 j83b_enable_size 		= (MT_U32)-1;
MT_U32 dvbc_enable_size			= (MT_U32)-1;
MT_U32 ephy_enable_size			= (MT_U32)-1;
MT_U32 hdr_enable_size			= (MT_U32)-1;
MT_U32 vdec10b_enable_size 		= (MT_U32)-1;
MT_U32 otp_smcd_enable_size 		= (MT_U32)-1;
MT_U32 otp_spimemmap_enable_size	= (MT_U32)-1;
MT_U32 otp_ethd_enable_size 		= (MT_U32)-1;

MT_U32 otp_vp9_enable			= (MT_U32)-1;
MT_U32 otp_vp9_enable_size 		= (MT_U32)-1;

/* SYM6 Added */
MT_U32 otp_rv_enable			= (MT_U32)-1;
MT_U32 otp_rv_enable_size 		= (MT_U32)-1;

MT_U32 otp_hdmi_emp_enable		= (MT_U32)-1;
MT_U32 otp_hdmi_emp_enable_size = (MT_U32)-1;

MT_U32 otp_hdmi_enable			= (MT_U32)-1;
MT_U32 otp_hdmi_enable_size 	= (MT_U32)-1;

MT_U32 otp_cpc_enable			= (MT_U32)-1;
MT_U32 otp_cpc_enable_size 		= (MT_U32)-1;

MT_U32 otp_armcore1_enable		= (MT_U32)-1;
MT_U32 otp_armcore1_enable_size = (MT_U32)-1;

MT_U32 otp_g31_enable			= (MT_U32)-1;
MT_U32 otp_g31_enable_size 		= (MT_U32)-1;

MT_U32 otp_usb1_30_enable		= (MT_U32)-1;
MT_U32 otp_usb1_30_enable_size 	= (MT_U32)-1;

MT_U32 otp_sl_hdr1_enable		= (MT_U32)-1;
MT_U32 otp_sl_hdr1_enable_size 	= (MT_U32)-1;

MT_U32 otp_hdr_hlg_enable		= (MT_U32)-1;
MT_U32 otp_hdr_hlg_enable_size 	= (MT_U32)-1;

MT_U32 otp_gmac_enable			= (MT_U32)-1;
MT_U32 otp_gmac_enable_size		= (MT_U32)-1;

MT_U32 otp_uhd_enable			= (MT_U32)-1;
MT_U32 otp_uhd_enable_size 		= (MT_U32)-1;


static void init_for_sym1(void)
{
	max_bit_addr		= (MT_U32)4096;		//sym1 otp has 4k bit addr
	chip_id_ca		= OTP_MAP_SYMPHONY_CHIP_ID_CA;
	ctrl_area_0		= OTP_MAP_SYMPHONY_CTRL_AREA_0;
	ctrl_area_size		= OTP_MAP_SYMPHONY_CTRL_AREA_SIZE;
	src_max_block_num 	= SYMPHONY_OTP_SRC_MAX_BLOCK_NUM;
	packet_type		= OTP_MAP_SYMPHONY_PACKET_TYPE;
	packet_type_size	= OTP_MAP_SYMPHONY_PACKET_TYPE_SIZE;
	chip_id			= OTP_MAP_SYMPHONY_CHIP_ID;
	hd_enable		= OTP_MAP_SYMPHONY_HD_ENABLE;
	hd_enable_size		= OTP_MAP_SYMPHONY_HD_ENABLE_SIZE;
	hd_dec_enable		= OTP_MAP_SYMPHONY_HD_DEC_ENABLE;
	hd_dec_enable_size	= OTP_MAP_SYMPHONY_HD_DEC_ENABLE_SIZE;
	dolby_enable		= OTP_MAP_SYMPHONY_DOLBY_ENABLE;
	dolby_enable_size	= OTP_MAP_SYMPHONY_DOLBY_ENABLE_SIZE;
	dra_enable		= OTP_MAP_SYMPHONY_DRA_ENABLE;
	dra_enable_size		= OTP_MAP_SYMPHONY_DRA_ENABLE_SIZE;
	ephy_enable		= OTP_MAP_SYMPHONY_EPHY_ENABLE;
	ephy_enable_size	= OTP_MAP_SYMPHONY_EPHY_ENABLE_SIZE;
	h265_enable		= OTP_MAP_SYMPHONY_H265_ENABLE;
	h265_enable_size	= OTP_MAP_SYMPHONY_H265_ENABLE_SIZE;
	cw_cfg_enable		= OTP_MAP_SYMPHONY_CW_CFG_ENABLE;
	cw_cfg_enable_size	= OTP_MAP_SYMPHONY_CW_CFG_ENABLE_SIZE;
	product			= (MT_U32)-1;
	product_size		= (MT_U32)-1;
	dram_cap		= (MT_U32)-1;
	dram_cap_size		= (MT_U32)-1;
	ddr_kgd			= (MT_U32)-1;
	ddr_kgd_size		= (MT_U32)-1;
}

static void init_for_sym2(void)
{
	max_bit_addr 		= 4096 * 32; 		//sym2 otp has 16k byte addr
	otp_casid			= OTP_MAP_SYMPHONY2_CASID;
	chip_id_ca		= OTP_MAP_SYMPHONY2_CHIP_ID_CA;
	ctrl_area_0		= (MT_U32)-1;
	ctrl_area_size		= 0;
	src_max_block_num 	= 0;
	packet_type		= OTP_MAP_SYMPHONY2_PACKET_TYPE;
	packet_type_size	= OTP_MAP_SYMPHONY2_PACKET_TYPE_SIZE;
	chip_id			= OTP_MAP_SYMPHONY2_CHIP_ID;
	hd_enable		= OTP_MAP_SYMPHONY2_HD_ENABLE;
	hd_enable_size		= OTP_MAP_SYMPHONY2_HD_ENABLE_SIZE;
	hd_dec_enable		= OTP_MAP_SYMPHONY2_HD_DEC_ENABLE;
	hd_dec_enable_size	= OTP_MAP_SYMPHONY2_HD_DEC_ENABLE_SIZE;
	dolby_enable		= OTP_MAP_SYMPHONY2_DOLBY_ENABLE;
	dolby_enable_size	= OTP_MAP_SYMPHONY2_DOLBY_ENABLE_SIZE;
	dra_enable		= OTP_MAP_SYMPHONY2_DRA_ENABLE;
	dra_enable_size		= OTP_MAP_SYMPHONY2_DRA_ENABLE_SIZE;
	ephy_enable		= OTP_MAP_SYMPHONY2_EPHY_ENABLE;
	ephy_enable_size	= OTP_MAP_SYMPHONY2_EPHY_ENABLE_SIZE;
	h265_enable		= OTP_MAP_SYMPHONY2_H265_ENABLE;
	h265_enable_size	= OTP_MAP_SYMPHONY2_H265_ENABLE_SIZE;
	cw_cfg_enable		= (MT_U32)-1;
	cw_cfg_enable_size	= 0;
	product			= OTP_MAP_SYMPHONY2_PRODUCT_TYPE;
	product_size		= OTP_MAP_SYMPHONY2_PRODUCT_TYPE_SIZE;
	dram_cap		= OTP_MAP_SYMPHONY2_DRAM_CAP;
	dram_cap_size		= OTP_MAP_SYMPHONY2_DRAM_CAP_SIZE;
	ddr_kgd			= OTP_MAP_SYMPHONY2_DDR_KGD;
	ddr_kgd_size		= OTP_MAP_SYMPHONY2_DDR_KGD_SIZE;

	display_resolution	= OTP_MAP_SYMPHONY2_DISPLAY_RESOLUTION;
	security		= OTP_MAP_SYMPHONY2_SECURITY_TYPE;
	package_info		= OTP_MAP_SYMPHONY2_PACKAGE_INFO;
	sipdramsize		= OTP_MAP_SYMPHONY2_SIP_DRAM_SIZE;
	ca_vendor		= OTP_MAP_SYMPHONY2_CA_VENDOR;
	drm_info		= OTP_MAP_SYMPHONY2_DRM_INFO;
	ip_license		= OTP_MAP_SYMPHONY2_IP_LICENSE;

	display_resolution_size	= OTP_MAP_SYMPHONY2_DISPLAY_RESOLUTION_SIZE;
	security_size		= OTP_MAP_SYMPHONY2_SECURITY_TYPE_SIZE;
	package_info_size	= OTP_MAP_SYMPHONY2_PACKAGE_INFO_SIZE;
	sipdramsize_size	= OTP_MAP_SYMPHONY2_SIP_DRAM_SIZE_SIZE;
	ca_vendor_size		= OTP_MAP_SYMPHONY2_CA_VENDOR_SIZE;
	drm_info_size		= OTP_MAP_SYMPHONY2_DRM_INFO_SIZE;
	ip_license_size		= OTP_MAP_SYMPHONY2_IP_LICENSE_SIZE;

	macrovision_enable		= OTP_MAP_SYMPHONY2_MACROVISION_ENABLE;
	usb0_enable			= OTP_MAP_SYMPHONY2_USB0_ENABLE;
	usb1_enable			= OTP_MAP_SYMPHONY2_USB1_ENABLE;
	dvbt2_enable			= OTP_MAP_SYMPHONY2_DVBT2_ENABLE;
	dvbt_enable			= OTP_MAP_SYMPHONY2_DVBT_ENABLE;
	dvbs2x_enable			= OTP_MAP_SYMPHONY2_DVBS2X_ENABLE;
	dvbs_s2_s2x_enable		= OTP_MAP_SYMPHONY2_DVBS_S2_S2X_ENABLE;
	j83b_enable			= OTP_MAP_SYMPHONY2_J83B_ENABLE;
	dvbc_enable			= OTP_MAP_SYMPHONY2_DVBC_ENABLE;
	hdr_enable			= OTP_MAP_SYMPHONY2_HDR_ENABLE;
	vdec10b_enable			= OTP_MAP_SYMPHONY2_VDEC10B_ENABLE;
	otp_smcd_enable			= OTP_MAP_SYMPHONY2_OTP_SMCD_ENABLE;
	otp_spimemmap_enable		= OTP_MAP_SYMPHONY2_OTP_SPIMEMMAP_ENABLE;
	otp_ethd_enable			= OTP_MAP_SYMPHONY2_OTP_ETHD_ENABLE;

	macrovision_enable_size 	= OTP_MAP_SYMPHONY2_MACROVISION_ENABLE_SIZE;
	usb0_enable_size 		= OTP_MAP_SYMPHONY2_USB0_ENABLE_SIZE;
	usb1_enable_size 		= OTP_MAP_SYMPHONY2_USB1_ENABLE_SIZE;
	dvbt2_enable_size 		= OTP_MAP_SYMPHONY2_DVBT2_ENABLE_SIZE;
	dvbt_enable_size 		= OTP_MAP_SYMPHONY2_DVBT_ENABLE_SIZE;
	dvbs2x_enable_size 		= OTP_MAP_SYMPHONY2_DVBS2X_ENABLE_SIZE;
	dvbs_s2_s2x_enable_size 	= OTP_MAP_SYMPHONY2_DVBS_S2_S2X_ENABLE_SIZE;
	j83b_enable_size 		= OTP_MAP_SYMPHONY2_J83B_ENABLE_SIZE;
	dvbc_enable_size		= OTP_MAP_SYMPHONY2_DVBC_ENABLE_SIZE;
	hdr_enable_size			= OTP_MAP_SYMPHONY2_HDR_ENABLE_SIZE;
	vdec10b_enable_size 		= OTP_MAP_SYMPHONY2_VDEC10B_ENABLE_SIZE;
	otp_smcd_enable_size 		= OTP_MAP_SYMPHONY2_OTP_SMCD_ENABLE_SIZE;
	otp_spimemmap_enable_size	= OTP_MAP_SYMPHONY2_OTP_SPIMEMMAP_ENABLE_SIZE;
	otp_ethd_enable_size 		= OTP_MAP_SYMPHONY2_OTP_ETHD_ENABLE_SIZE;

}

static void init_for_sym4(void)
{
	max_bit_addr 		= 4096 * 32; 		//sym4 otp has 16k byte addr
	otp_casid			= (MT_U32)-1;
	chip_id_ca		= OTP_MAP_SYMPHONY4_CHIP_ID_CA;
	ctrl_area_0		= (MT_U32)-1;
	ctrl_area_size		= 0;
	src_max_block_num 	= 0;
	packet_type		= OTP_MAP_SYMPHONY4_PACKAGE_INFO;
	packet_type_size	= OTP_MAP_SYMPHONY4_PACKAGE_INFO_SIZE;
	chip_id			= OTP_MAP_SYMPHONY4_CHIP_ID;
	hd_enable		= OTP_MAP_SYMPHONY4_HD_ENABLE;
	hd_enable_size		= OTP_MAP_SYMPHONY4_HD_ENABLE_SIZE;
	hd_dec_enable		= (MT_U32)-1;
	hd_dec_enable_size	= (MT_U32)-1;
	dolby_enable		= OTP_MAP_SYMPHONY4_IP_LICENSE;
	dolby_enable_size	= OTP_MAP_SYMPHONY4_IP_LICENSE_SIZE;
	dra_enable			= OTP_MAP_SYMPHONY4_DRA_ENABLE;
	dra_enable_size		= OTP_MAP_SYMPHONY4_DRA_ENABLE_SIZE;
	ephy_enable		= OTP_MAP_SYMPHONY4_EPHY_ENABLE;
	ephy_enable_size	= OTP_MAP_SYMPHONY4_EPHY_ENABLE_SIZE;
	h265_enable		= OTP_MAP_SYMPHONY4_H265_ENABLE;
	h265_enable_size	= OTP_MAP_SYMPHONY4_H265_ENABLE_SIZE;
	cw_cfg_enable		= (MT_U32)-1;
	cw_cfg_enable_size	= 0;
	product			= OTP_MAP_SYMPHONY4_PRODUCT_TYPE;
	product_size		= OTP_MAP_SYMPHONY4_PRODUCT_TYPE_SIZE;
	dram_cap		= (MT_U32)-1;
	dram_cap_size		= (MT_U32)-1;
	ddr_kgd			= OTP_MAP_SYMPHONY4_DDR_KGD;
	ddr_kgd_size		= OTP_MAP_SYMPHONY4_DDR_KGD_SIZE;

	display_resolution	= OTP_MAP_SYMPHONY4_DISPLAY_RESOLUTION;
	security		= OTP_MAP_SYMPHONY4_SECURITY_TYPE;
	package_info		= OTP_MAP_SYMPHONY4_PACKAGE_INFO;
	sipdramsize		= OTP_MAP_SYMPHONY4_SIP_DRAM_SIZE;
	ca_vendor		= OTP_MAP_SYMPHONY4_CA_VENDOR;
	ca_version		= OTP_MAP_SYMPHONY4_CA_VERSION;
	drm_info		= OTP_MAP_SYMPHONY4_DRM_INFO;
	ip_license		= OTP_MAP_SYMPHONY4_IP_LICENSE;

	display_resolution_size	= OTP_MAP_SYMPHONY4_DISPLAY_RESOLUTION_SIZE;
	security_size		= OTP_MAP_SYMPHONY4_SECURITY_TYPE_SIZE;
	package_info_size	= OTP_MAP_SYMPHONY4_PACKAGE_INFO_SIZE;
	sipdramsize_size	= OTP_MAP_SYMPHONY4_SIP_DRAM_SIZE_SIZE;
	ca_vendor_size		= OTP_MAP_SYMPHONY4_CA_VENDOR_SIZE;
	ca_version_size		= OTP_MAP_SYMPHONY4_CA_VERSION_SIZE;
	drm_info_size		= OTP_MAP_SYMPHONY4_DRM_INFO_SIZE;
	ip_license_size		= OTP_MAP_SYMPHONY4_IP_LICENSE_SIZE;

	macrovision_enable		= OTP_MAP_SYMPHONY4_MACROVISION_ENABLE;
	usb0_enable			= OTP_MAP_SYMPHONY4_USB0_ENABLE;
	usb1_enable			= OTP_MAP_SYMPHONY4_USB1_ENABLE;
	dvbt2_enable			= (MT_U32)-1;
	dvbt_enable			= (MT_U32)-1;
	dvbs2x_enable			= OTP_MAP_SYMPHONY4_DVBS2X_ENABLE;
	dvbs_s2_s2x_enable		= OTP_MAP_SYMPHONY4_DVBS_ENABLE;
	j83b_enable			= OTP_MAP_SYMPHONY4_J83B_ENABLE;
	dvbc_enable			= OTP_MAP_SYMPHONY4_DVBC_ENABLE;
	hdr_enable			= OTP_MAP_SYMPHONY4_HDR_ENABLE;
	vdec10b_enable			= OTP_MAP_SYMPHONY4_VDEC10B_ENABLE;
	otp_smcd_enable			= OTP_MAP_SYMPHONY4_OTP_SMCD_ENABLE;
	otp_spimemmap_enable		= (MT_U32)-1;
	otp_ethd_enable			= OTP_MAP_SYMPHONY4_OTP_ETHD_ENABLE;

	macrovision_enable_size 	= OTP_MAP_SYMPHONY4_MACROVISION_ENABLE_SIZE;
	usb0_enable_size 		= OTP_MAP_SYMPHONY4_USB0_ENABLE_SIZE;
	usb1_enable_size 		= OTP_MAP_SYMPHONY4_USB1_ENABLE_SIZE;
	dvbt2_enable_size 		= (MT_U32)-1;
	dvbt_enable_size 		= (MT_U32)-1;
	dvbs2x_enable_size 		= OTP_MAP_SYMPHONY4_DVBS2X_ENABLE_SIZE;
	dvbs_s2_s2x_enable_size 	= OTP_MAP_SYMPHONY4_DVBS_ENABLE_SIZE;
	j83b_enable_size 		= OTP_MAP_SYMPHONY4_J83B_ENABLE_SIZE;
	dvbc_enable_size		= OTP_MAP_SYMPHONY4_DVBC_ENABLE_SIZE;
	hdr_enable_size			= OTP_MAP_SYMPHONY4_HDR_ENABLE_SIZE;
	vdec10b_enable_size 		= OTP_MAP_SYMPHONY4_VDEC10B_ENABLE_SIZE;
	otp_smcd_enable_size 		= OTP_MAP_SYMPHONY4_OTP_SMCD_ENABLE_SIZE;
	otp_spimemmap_enable_size	= (MT_U32)-1;
	otp_ethd_enable_size 		= OTP_MAP_SYMPHONY4_OTP_ETHD_ENABLE_SIZE;

	family					= OTP_MAP_SYMPHONY4_CHIP_FAMILY;
	family_size				= OTP_MAP_SYMPHONY4_CHIP_FAMILY_SIZE;

	generation				= OTP_MAP_SYMPHONY4_CHIP_GENERATION;
	generation_size			= OTP_MAP_SYMPHONY4_CHIP_GENERATION_SIZE;

	otp_vp9_enable			= OTP_MAP_SYMPHONY4_OTP_VP9_ENABLE;
	otp_vp9_enable_size		= OTP_MAP_SYMPHONY4_OTP_VP9_ENABLE_SIZE;

}

static void init_for_sym6(void)
{
	max_bit_addr 				= 4096 * 32; 		/* sym6 otp has 16k byte addr */
	otp_casid					= (MT_U32)-1;
	chip_id_ca					= OTP_MAP_SYMPHONY6_CAS_CHIP_ID;
	ctrl_area_0					= (MT_U32)-1;
	ctrl_area_size				= 0;
	src_max_block_num 			= 0;

	packet_type					= OTP_MAP_SYMPHONY6_PACKAGE_INFO;
	packet_type_size			= OTP_MAP_SYMPHONY6_PACKAGE_INFO_SIZE;

	chip_id						= OTP_MAP_SYMPHONY6_CHIP_ID;

	hd_enable					= (MT_U32)-1;
	hd_enable_size				= (MT_U32)-1;

	hd_dec_enable				= (MT_U32)-1;
	hd_dec_enable_size			= (MT_U32)-1;

	dolby_enable				= OTP_MAP_SYMPHONY6_IP_LICENSE;
	dolby_enable_size			= OTP_MAP_SYMPHONY6_IP_LICENSE_SIZE;

	dra_enable					= OTP_MAP_SYMPHONY6_DRA_ENABLE;
	dra_enable_size				= OTP_MAP_SYMPHONY6_DRA_ENABLE_SIZE;

	ephy_enable					= OTP_MAP_SYMPHONY6_EMAC_DIS;
	ephy_enable_size			= OTP_MAP_SYMPHONY6_EMAC_DIS_SIZE;

	h265_enable					= OTP_MAP_SYMPHONY6_H265_DIS;
	h265_enable_size			= OTP_MAP_SYMPHONY6_H265_DIS_SIZE;

	cw_cfg_enable				= (MT_U32)-1;
	cw_cfg_enable_size			= 0;

	product						= OTP_MAP_SYMPHONY6_PRODUCT_TYPE;
	product_size				= OTP_MAP_SYMPHONY6_PRODUCT_TYPE_SIZE;

	dram_cap					= (MT_U32)-1;
	dram_cap_size				= (MT_U32)-1;

	ddr_kgd						= (MT_U32)-1;
	ddr_kgd_size				= (MT_U32)-1;

	display_resolution			= OTP_MAP_SYMPHONY6_DISPLAY_RESOLUTION;
	display_resolution_size		= OTP_MAP_SYMPHONY6_DISPLAY_RESOLUTION_SIZE;

	security					= (MT_U32)-1;
	security_size				= (MT_U32)-1;

	package_info				= OTP_MAP_SYMPHONY6_PACKAGE_INFO;
	package_info_size			= OTP_MAP_SYMPHONY6_PACKAGE_INFO_SIZE;

	sipdramsize					= OTP_MAP_SYMPHONY6_SIP_DRAM_SIZE;
	sipdramsize_size			= OTP_MAP_SYMPHONY6_SIP_DRAM_SIZE_SIZE;

	ca_vendor					= OTP_MAP_SYMPHONY6_CA_VENDOR;
	ca_vendor_size				= OTP_MAP_SYMPHONY6_CA_VENDOR_SIZE;

	ca_version					= (MT_U32)-1;
	ca_version_size				= (MT_U32)-1;

	drm_info					= (MT_U32)-1;
	drm_info_size				= (MT_U32)-1;

	ip_license					= OTP_MAP_SYMPHONY6_IP_LICENSE;
	ip_license_size				= OTP_MAP_SYMPHONY6_IP_LICENSE_SIZE;

	macrovision_enable			= OTP_MAP_SYMPHONY6_MACROVISION_DIS;
	macrovision_enable_size 	= OTP_MAP_SYMPHONY6_MACROVISION_DIS_SIZE;

	usb0_enable					= OTP_MAP_SYMPHONY6_USB0_DIS;
	usb0_enable_size 			= OTP_MAP_SYMPHONY6_USB0_DIS_SIZE;

	usb1_enable					= OTP_MAP_SYMPHONY6_USB1_DIS;
	usb1_enable_size 			= OTP_MAP_SYMPHONY6_USB1_DIS_SIZE;

	dvbt2_enable				= (MT_U32)-1;
	dvbt2_enable_size 			= (MT_U32)-1;

	dvbt_enable					= OTP_MAP_SYMPHONY6_DVBT_DIS;
	dvbt_enable_size 			= OTP_MAP_SYMPHONY6_DVBT_DIS_SIZE;

	dvbs2x_enable				= OTP_MAP_SYMPHONY6_DVBS2X_DIS;
	dvbs2x_enable_size 			= OTP_MAP_SYMPHONY6_DVBS2X_DIS_SIZE;

	/* DVBS/S2 */
	dvbs_s2_s2x_enable			= OTP_MAP_SYMPHONY6_DVBS_S2_DIS;
	dvbs_s2_s2x_enable_size		= OTP_MAP_SYMPHONY6_DVBS_S2_DIS_SIZE;

	j83b_enable					= OTP_MAP_SYMPHONY6_J83B_DIS;
	j83b_enable_size 			= OTP_MAP_SYMPHONY6_J83B_DIS_SIZE;

	dvbc_enable					= OTP_MAP_SYMPHONY6_DVBC_DIS;
	dvbc_enable_size			= OTP_MAP_SYMPHONY6_DVBC_DIS_SIZE;

	hdr_enable					= OTP_MAP_SYMPHONY6_HDR10P_DIS;
	hdr_enable_size				= OTP_MAP_SYMPHONY6_HDR10P_DIS_SIZE;

	vdec10b_enable				= OTP_MAP_SYMPHONY6_VDEC10B_DIS;
	vdec10b_enable_size 		= OTP_MAP_SYMPHONY6_VDEC10B_DIS_SIZE;

	otp_smcd_enable		 		= OTP_MAP_SYMPHONY6_SMC_DIS;
	otp_smcd_enable_size 		= OTP_MAP_SYMPHONY6_SMC_DIS_SIZE;

	otp_spimemmap_enable 		= (MT_U32)-1;
	otp_spimemmap_enable_size	= (MT_U32)-1;

	otp_ethd_enable		 		= OTP_MAP_SYMPHONY6_EMAC_DIS;
	otp_ethd_enable_size 		= OTP_MAP_SYMPHONY6_EMAC_DIS_SIZE;

	family						= OTP_MAP_SYMPHONY6_CHIP_FAMILY;
	family_size					= OTP_MAP_SYMPHONY6_CHIP_FAMILY_SIZE;

	generation					= OTP_MAP_SYMPHONY6_CHIP_GENERATION;
	generation_size				= OTP_MAP_SYMPHONY6_CHIP_GENERATION_SIZE;

	otp_vp9_enable				= OTP_MAP_SYMPHONY6_VP9_DIS;
	otp_vp9_enable_size			= OTP_MAP_SYMPHONY6_VP9_DIS_SIZE;

	otp_rv_enable				= OTP_MAP_SYMPHONY6_RV_DIS;
	otp_rv_enable_size			= OTP_MAP_SYMPHONY6_RV_DIS_SIZE;

	otp_hdmi_emp_enable			= OTP_MAP_SYMPHONY6_HDMI_EMP_DIS;
	otp_hdmi_emp_enable_size	= OTP_MAP_SYMPHONY6_HDMI_EMP_DIS_SIZE;

	otp_hdmi_enable				= OTP_MAP_SYMPHONY6_HDMI_DIS;
	otp_hdmi_enable_size		= OTP_MAP_SYMPHONY6_HDMI_DIS_SIZE;

	otp_cpc_enable 				= OTP_MAP_SYMPHONY6_CPC_DIS;
	otp_cpc_enable_size			= OTP_MAP_SYMPHONY6_CPC_DIS_SIZE;

	otp_armcore1_enable			= OTP_MAP_SYMPHONY6_ARM_CORE1_DIS;
	otp_armcore1_enable_size 	= OTP_MAP_SYMPHONY6_ARM_CORE1_DIS_SIZE;

	otp_g31_enable				= OTP_MAP_SYMPHONY6_G31_DIS;
	otp_g31_enable_size 		= OTP_MAP_SYMPHONY6_G31_DIS_SIZE;

	otp_usb1_30_enable			= OTP_MAP_SYMPHONY6_USB1_30_DIS;
	otp_usb1_30_enable_size 	= OTP_MAP_SYMPHONY6_USB1_30_DIS_SIZE;

	otp_sl_hdr1_enable			= OTP_MAP_SYMPHONY6_SL_HDR1_DIS;
	otp_sl_hdr1_enable_size 	= OTP_MAP_SYMPHONY6_SL_HDR1_DIS_SIZE;

	otp_hdr_hlg_enable			= OTP_MAP_SYMPHONY6_HDR10_HLG_DIS;
	otp_hdr_hlg_enable_size 	= OTP_MAP_SYMPHONY6_HDR10_HLG_DIS_SIZE;

	otp_gmac_enable				= OTP_MAP_SYMPHONY6_GMAC_DIS;
	otp_gmac_enable_size 		= OTP_MAP_SYMPHONY6_GMAC_DIS_SIZE;

	otp_uhd_enable 				= OTP_MAP_SYMPHONY6_UHD_DIS;
	otp_uhd_enable_size			= OTP_MAP_SYMPHONY6_UHD_DIS_SIZE;

}

static void init_for_sym6_a1(void)
{
	max_bit_addr 				= 4096 * 32; 		/* sym6 otp has 16k byte addr */
	otp_casid					= (MT_U32)-1;
	chip_id_ca					= OTP_MAP_SYMPHONY6_CAS_CHIP_ID;
	ctrl_area_0					= (MT_U32)-1;
	ctrl_area_size				= 0;
	src_max_block_num 			= 0;

	packet_type					= OTP_MAP_SYMPHONY6_PACKAGE_INFO;
	packet_type_size			= OTP_MAP_SYMPHONY6_PACKAGE_INFO_SIZE;

	chip_id						= OTP_MAP_SYMPHONY6_CHIP_ID;

	hd_enable					= (MT_U32)-1;
	hd_enable_size				= (MT_U32)-1;

	hd_dec_enable				= (MT_U32)-1;
	hd_dec_enable_size			= (MT_U32)-1;

	dolby_enable				= OTP_MAP_SYMPHONY6_IP_LICENSE;
	dolby_enable_size			= OTP_MAP_SYMPHONY6_IP_LICENSE_SIZE;

	dra_enable					= OTP_MAP_SYMPHONY6_DRA_ENABLE;
	dra_enable_size				= OTP_MAP_SYMPHONY6_DRA_ENABLE_SIZE;

	ephy_enable					= OTP_MAP_SYMPHONY6_EMAC_DIS;
	ephy_enable_size			= OTP_MAP_SYMPHONY6_EMAC_DIS_SIZE;

	h265_enable					= OTP_MAP_SYMPHONY6_H265_DIS;
	h265_enable_size			= OTP_MAP_SYMPHONY6_H265_DIS_SIZE;

	cw_cfg_enable				= (MT_U32)-1;
	cw_cfg_enable_size			= 0;

	product						= OTP_MAP_SYMPHONY6_PRODUCT_TYPE;
	product_size				= OTP_MAP_SYMPHONY6_PRODUCT_TYPE_SIZE;

	dram_cap					= (MT_U32)-1;
	dram_cap_size				= (MT_U32)-1;

	ddr_kgd						= (MT_U32)-1;
	ddr_kgd_size				= (MT_U32)-1;

	display_resolution			= OTP_MAP_SYMPHONY6_DISPLAY_RESOLUTION;
	display_resolution_size		= OTP_MAP_SYMPHONY6_DISPLAY_RESOLUTION_SIZE;

	security					= (MT_U32)-1;
	security_size				= (MT_U32)-1;

	package_info				= OTP_MAP_SYMPHONY6_PACKAGE_INFO;
	package_info_size			= OTP_MAP_SYMPHONY6_PACKAGE_INFO_SIZE;

/* A0 */
//	sipdramsize					= OTP_MAP_SYMPHONY6_SIP_DRAM_SIZE;
//	sipdramsize_size			= OTP_MAP_SYMPHONY6_SIP_DRAM_SIZE_SIZE;
/* A1 */
	sipdramsize					= OTP_MAP_SYMPHONY6_A1_SIP_DRAM_SIZE;
	sipdramsize_size			= OTP_MAP_SYMPHONY6_A1_SIP_DRAM_SIZE_SIZE;

/* A0 */
//	ca_vendor					= OTP_MAP_SYMPHONY6_CA_VENDOR;
//	ca_vendor_size				= OTP_MAP_SYMPHONY6_CA_VENDOR_SIZE;
/* A1 */
	ca_vendor					= OTP_MAP_SYMPHONY6_A1_CA_VENDOR;
	ca_vendor_size				= OTP_MAP_SYMPHONY6_A1_CA_VENDOR_SIZE;

/* A1 */
	ca_version					= OTP_MAP_SYMPHONY6_CHIPSET_VERSION;
	ca_version_size				= OTP_MAP_SYMPHONY6_CHIPSET_VERSION_SIZE;

	drm_info					= (MT_U32)-1;
	drm_info_size				= (MT_U32)-1;

	ip_license					= OTP_MAP_SYMPHONY6_IP_LICENSE;
	ip_license_size				= OTP_MAP_SYMPHONY6_IP_LICENSE_SIZE;

	macrovision_enable			= OTP_MAP_SYMPHONY6_MACROVISION_DIS;
	macrovision_enable_size 	= OTP_MAP_SYMPHONY6_MACROVISION_DIS_SIZE;

	usb0_enable					= OTP_MAP_SYMPHONY6_USB0_DIS;
	usb0_enable_size 			= OTP_MAP_SYMPHONY6_USB0_DIS_SIZE;

	usb1_enable					= OTP_MAP_SYMPHONY6_USB1_DIS;
	usb1_enable_size 			= OTP_MAP_SYMPHONY6_USB1_DIS_SIZE;

	dvbt2_enable				= (MT_U32)-1;
	dvbt2_enable_size 			= (MT_U32)-1;

/* A0 */
//	dvbt_enable					= OTP_MAP_SYMPHONY6_DVBT_DIS;
//	dvbt_enable_size 			= OTP_MAP_SYMPHONY6_DVBT_DIS_SIZE;
/* A1 */
	dvbt_enable 				= (MT_U32)-1;
	dvbt_enable_size			= (MT_U32)-1;

	dvbs2x_enable				= OTP_MAP_SYMPHONY6_DVBS2X_DIS;
	dvbs2x_enable_size 			= OTP_MAP_SYMPHONY6_DVBS2X_DIS_SIZE;

	/* DVBS/S2 */
	dvbs_s2_s2x_enable			= OTP_MAP_SYMPHONY6_DVBS_S2_DIS;
	dvbs_s2_s2x_enable_size		= OTP_MAP_SYMPHONY6_DVBS_S2_DIS_SIZE;

	j83b_enable					= OTP_MAP_SYMPHONY6_J83B_DIS;
	j83b_enable_size 			= OTP_MAP_SYMPHONY6_J83B_DIS_SIZE;

	dvbc_enable					= OTP_MAP_SYMPHONY6_DVBC_DIS;
	dvbc_enable_size			= OTP_MAP_SYMPHONY6_DVBC_DIS_SIZE;

	hdr_enable					= OTP_MAP_SYMPHONY6_HDR10P_DIS;
	hdr_enable_size				= OTP_MAP_SYMPHONY6_HDR10P_DIS_SIZE;

	vdec10b_enable				= OTP_MAP_SYMPHONY6_VDEC10B_DIS;
	vdec10b_enable_size 		= OTP_MAP_SYMPHONY6_VDEC10B_DIS_SIZE;

	otp_smcd_enable		 		= OTP_MAP_SYMPHONY6_SMC_DIS;
	otp_smcd_enable_size 		= OTP_MAP_SYMPHONY6_SMC_DIS_SIZE;

	otp_spimemmap_enable 		= (MT_U32)-1;
	otp_spimemmap_enable_size	= (MT_U32)-1;

	otp_ethd_enable		 		= OTP_MAP_SYMPHONY6_EMAC_DIS;
	otp_ethd_enable_size 		= OTP_MAP_SYMPHONY6_EMAC_DIS_SIZE;

	family						= OTP_MAP_SYMPHONY6_CHIP_FAMILY;
	family_size					= OTP_MAP_SYMPHONY6_CHIP_FAMILY_SIZE;

	generation					= OTP_MAP_SYMPHONY6_CHIP_GENERATION;
	generation_size				= OTP_MAP_SYMPHONY6_CHIP_GENERATION_SIZE;

	otp_vp9_enable				= OTP_MAP_SYMPHONY6_VP9_DIS;
	otp_vp9_enable_size			= OTP_MAP_SYMPHONY6_VP9_DIS_SIZE;

	otp_rv_enable				= OTP_MAP_SYMPHONY6_RV_DIS;
	otp_rv_enable_size			= OTP_MAP_SYMPHONY6_RV_DIS_SIZE;

	otp_hdmi_emp_enable			= OTP_MAP_SYMPHONY6_HDMI_EMP_DIS;
	otp_hdmi_emp_enable_size	= OTP_MAP_SYMPHONY6_HDMI_EMP_DIS_SIZE;

	otp_hdmi_enable				= OTP_MAP_SYMPHONY6_HDMI_DIS;
	otp_hdmi_enable_size		= OTP_MAP_SYMPHONY6_HDMI_DIS_SIZE;

	otp_cpc_enable 				= OTP_MAP_SYMPHONY6_CPC_DIS;
	otp_cpc_enable_size			= OTP_MAP_SYMPHONY6_CPC_DIS_SIZE;

	otp_armcore1_enable			= OTP_MAP_SYMPHONY6_ARM_CORE1_DIS;
	otp_armcore1_enable_size 	= OTP_MAP_SYMPHONY6_ARM_CORE1_DIS_SIZE;

	otp_g31_enable				= OTP_MAP_SYMPHONY6_G31_DIS;
	otp_g31_enable_size 		= OTP_MAP_SYMPHONY6_G31_DIS_SIZE;

	otp_usb1_30_enable			= OTP_MAP_SYMPHONY6_USB1_30_DIS;
	otp_usb1_30_enable_size 	= OTP_MAP_SYMPHONY6_USB1_30_DIS_SIZE;

	otp_sl_hdr1_enable			= OTP_MAP_SYMPHONY6_SL_HDR1_DIS;
	otp_sl_hdr1_enable_size 	= OTP_MAP_SYMPHONY6_SL_HDR1_DIS_SIZE;

	otp_hdr_hlg_enable			= OTP_MAP_SYMPHONY6_HDR10_HLG_DIS;
	otp_hdr_hlg_enable_size 	= OTP_MAP_SYMPHONY6_HDR10_HLG_DIS_SIZE;

	otp_gmac_enable				= OTP_MAP_SYMPHONY6_GMAC_DIS;
	otp_gmac_enable_size 		= OTP_MAP_SYMPHONY6_GMAC_DIS_SIZE;

	otp_uhd_enable 				= OTP_MAP_SYMPHONY6_UHD_DIS;
	otp_uhd_enable_size			= OTP_MAP_SYMPHONY6_UHD_DIS_SIZE;

}

MT_S32 MT_UNF_OTP_Init(MT_VOID)
{
	mt_sys_version_s chip_version = {0};

	if (OtpHandle != -1)
		return MT_SUCCESS;

	OtpHandle = open ("/dev/mt_otp",  O_RDWR);
	if (OtpHandle < 0)
	{
		OtpHandle = -1;
		MT_ERR_OTP("MT_UNF_OTP_Init: open dev failed!\n");;
		return MT_FAILURE;
	}

	(void)mt_sys_get_version(&chip_version);

	otp_version = chip_version.enChipVersion;

	if (mt_chip_is_symphony1(otp_version) || otp_version == MT_CHIP_SYMPHONY3_A0)
	{
		init_for_sym1();
	}
	else if (mt_chip_is_symphony2(otp_version))
	{
		init_for_sym2();
	}
	else if (mt_chip_is_symphony4(otp_version))
	{
		init_for_sym4();
	}
	else if (mt_chip_is_symphony6(otp_version))
	{
		if (MT_CHIP_SYMPHONY6_A1 == otp_version)
			init_for_sym6_a1();
		else
			init_for_sym6();
	}
	else
	{
		MT_ERR_OTP("MT_UNF_OTP_Init: unknown chip verison: 0x%X!\n", otp_version);;
		close(OtpHandle);
		OtpHandle = -1;
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

MT_S32 MT_UNF_OTP_Deinit(MT_VOID)
{
	if (OtpHandle != -1)
	{
		close(OtpHandle);
		OtpHandle = -1;
	}

	return MT_SUCCESS;
}

#define CHECK_DEV_OPEN_RETURN				\
				do {						\
					if (OtpHandle == -1) {	\
						MT_ERR_OTP("dev is not opened!\n");	\
						return MT_FAILURE;	\
					}						\
				} while(0)

#define CHECK_NULL_PTR_RETURN(ptr)			\
				do {						\
					if(NULL == ptr) {		\
						MT_ERR_OTP("args pointer is null!\n");	\
						return MT_FAILURE;	\
					}						\
				} while(0)

#define CHECK_BITADDR_RETURN(bitaddr)		\
				do {						\
					if (bitaddr >= max_bit_addr) {	\
						MT_ERR_OTP("bitaddr largs than (%u)\n", max_bit_addr - 1);	\
						return MT_FAILURE;	\
					}						\
				} while(0)

#define CHECK_BITLEN_RETURN(bitlen)			\
				do {						\
					if ((0 == bitlen) || (bitlen > 32)) {	\
						MT_ERR_OTP("bitlen equal 0 or largs than 32 \n");	\
						return MT_FAILURE;	\
					}						\
				} while(0)

MT_S32 MT_UNF_OTP_read(MT_U32 bitaddr, MT_U32 bitlen, MT_U32 *data)
{
	int res;
	struct otp_transfer_data trandata;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(data);
	CHECK_BITADDR_RETURN(bitaddr);
	CHECK_BITLEN_RETURN(bitlen);

	trandata.bit_addr = bitaddr;
	trandata.bit_len = bitlen;
	trandata.buf_size = 4;
	trandata.buf = (void *)data;

	res = ioctl(OtpHandle, OTP_DATA_OUT, &trandata);
	if (res < 0)
	{
		perror("ioctl");
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

MT_S32 MT_UNF_OTP_write(MT_U32 bitaddr, MT_U32 bitlen, MT_U32 data)
{
	int res;
	struct otp_transfer_data trandata;

	CHECK_DEV_OPEN_RETURN;
	CHECK_BITADDR_RETURN(bitaddr);
	CHECK_BITLEN_RETURN(bitlen);

	trandata.bit_addr = bitaddr;
	trandata.bit_len = bitlen;
	trandata.buf_size = 4;
	trandata.buf = (void *)&data;

	res = ioctl(OtpHandle, OTP_DATA_IN, &trandata);
	if (res < 0)
	{
		perror("ioctl");
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

MT_S32 MT_UNF_OTP_unlock(MT_U32 bitaddr, MT_U32 bitlen, MT_U32 data)
{
	int res;
	struct otp_transfer_data trandata;

	CHECK_DEV_OPEN_RETURN;
	CHECK_BITADDR_RETURN(bitaddr);
	CHECK_BITLEN_RETURN(bitlen);

	trandata.bit_addr = bitaddr;
	trandata.bit_len = bitlen;
	trandata.buf_size = 4;
	trandata.buf = (void *)&data;

	res = ioctl(OtpHandle,OTP_DATA_UNLOCK,&trandata);
	if (res < 0)
	{
		perror("ioctl");
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

//MT_S32 MT_UNF_OTP_disable_cw_cfg(void)
//{
//	return MT_UNF_OTP_write(OTP_MAP_SYMPHONY_CW_CFG_ENABLE, OTP_MAP_SYMPHONY_CW_CFG_ENABLE_SIZE, 1);
//}

/* CA Chip ID */
MT_S32 MT_UNF_OTP_get_pid(MT_U32 *pid_h, MT_U32 *pid_l)
{
	MT_U32 tmp = 0;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(pid_h);
	CHECK_NULL_PTR_RETURN(pid_l);

	if (mt_chip_is_symphony1(otp_version) || otp_version == MT_CHIP_SYMPHONY3_A0)
	{
		if(MT_UNF_OTP_read(chip_id_ca, 32, pid_l))
			return MT_FAILURE;
		if(MT_UNF_OTP_read(chip_id_ca + 32, 32, pid_h))
			return MT_FAILURE;
		return MT_SUCCESS;
	}
	else if (mt_chip_is_symphony2(otp_version))
	{
		if(MT_UNF_OTP_read(otp_casid,8,&tmp))
			return MT_FAILURE;
		if(0xff == tmp)
			return MT_FAILURE;
		if(MT_UNF_OTP_read(chip_id_ca,32,pid_l) || MT_UNF_OTP_read(chip_id_ca + 32,32,pid_h))
			return MT_FAILURE;
		return MT_SUCCESS;
	}
	else if (mt_chip_is_symphony4(otp_version))
	{
		if(MT_UNF_OTP_read(chip_id_ca,32,pid_l) || MT_UNF_OTP_read(chip_id_ca + 32,32,pid_h))
			return MT_FAILURE;
		return MT_SUCCESS;
	}
	else if (mt_chip_is_symphony6(otp_version))
	{
		if(MT_UNF_OTP_read(chip_id_ca,32,pid_l) || MT_UNF_OTP_read(chip_id_ca + 32,32,pid_h))
			return MT_FAILURE;
		return MT_SUCCESS;
	}

	return MT_FAILURE;
}

MT_S32 MT_UNF_OTP_get_product(otp_product_type_t * data)
{
	MT_U32 tmp;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(data);

	if (product == -1)
		return MT_FAILURE;

	if(MT_UNF_OTP_read(product, product_size, &tmp))
		return MT_FAILURE;

	*data = (otp_product_type_t)tmp;

	return MT_SUCCESS;
}


MT_S32 MT_UNF_OTP_get_chip_family(int * data)
{
	MT_U32 tmp;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(data);

	if (family == -1)
		return MT_FAILURE;

	if(MT_UNF_OTP_read(family, family_size, &tmp))
		return MT_FAILURE;

	*data = (int)tmp;

	return MT_SUCCESS;
}


MT_S32 MT_UNF_OTP_get_chip_generation(int * data)
{
	MT_U32 tmp;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(data);

	if (generation == -1)
		return MT_FAILURE;

	if(MT_UNF_OTP_read(generation, generation_size, &tmp))
		return MT_FAILURE;

	switch(tmp){
		case 0:
			*data = 2;	/* Symphony2 */
			break;
		case 1:
			*data = 4;	/* Symphony4 */
			break;
		case 2:
			*data = 6;	/* Symphony6 */
			break;
		default:
			return MT_FAILURE;
	}
	return MT_SUCCESS;
}


MT_S32 MT_UNF_OTP_get_display_resolution(otp_display_resolution_type_t * data)
{
	MT_U32 tmp;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(data);

	if (display_resolution == -1)
		return MT_FAILURE;

	if(MT_UNF_OTP_read(display_resolution, display_resolution_size, &tmp))
		return MT_FAILURE;

	*data = (otp_display_resolution_type_t)tmp;
	return MT_SUCCESS;
}


MT_S32 MT_UNF_OTP_get_secure_type(otp_security_type_t * data)
{
	MT_U32 tmp;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(data);

	if (security == -1)
		return MT_FAILURE;

	if(MT_UNF_OTP_read(security, security_size, &tmp))
		return MT_FAILURE;
	*data = (otp_security_type_t)tmp;
	return MT_SUCCESS;
}


MT_S32 MT_UNF_OTP_get_package_info(otp_package_info_type_t * data)
{
	MT_U32 tmp;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(data);

	if (package_info == -1)
		return MT_FAILURE;

	if(MT_UNF_OTP_read(package_info, package_info_size, &tmp))
		return MT_FAILURE;
	*data = (otp_package_info_type_t)tmp;
	return MT_SUCCESS;
}


MT_S32 MT_UNF_OTP_get_sipdramsize(otp_sip_dram_size_type_t * data)
{
	MT_U32 tmp;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(data);

	if (sipdramsize == -1)
		return MT_FAILURE;

	if(MT_UNF_OTP_read(sipdramsize, sipdramsize_size, &tmp))
		return MT_FAILURE;
	*data = (otp_sip_dram_size_type_t)tmp;
	return MT_SUCCESS;
}


MT_S32 MT_UNF_OTP_get_ca_vendor(otp_ca_vendor_type_t * data)
{
	MT_U32 tmp;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(data);

	if (ca_vendor == -1)
		return MT_FAILURE;

	if(MT_UNF_OTP_read(ca_vendor, ca_vendor_size, &tmp))
		return MT_FAILURE;
	*data = (otp_ca_vendor_type_t)tmp;
	return MT_SUCCESS;
}


MT_S32 MT_UNF_OTP_get_ca_version(int * data)
{
	MT_U32 tmp;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(data);

	if (ca_version == -1)
		return MT_FAILURE;

	if(MT_UNF_OTP_read(ca_version, ca_version_size, &tmp))
		return MT_FAILURE;
	*data = (int)tmp;
	return MT_SUCCESS;
}


MT_S32 MT_UNF_OTP_get_drm_info(otp_drm_info_type_t * data)
{
	MT_U32 tmp;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(data);

	if (drm_info == -1)
		return MT_FAILURE;

	if(MT_UNF_OTP_read(drm_info, drm_info_size, &tmp))
		return MT_FAILURE;
	*data = (otp_drm_info_type_t)tmp;
	return MT_SUCCESS;
}


MT_S32 MT_UNF_OTP_get_ip_license(otp_ip_license_type_t * data)
{
	MT_U32 tmp;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(data);

	if (ip_license == -1)
		return MT_FAILURE;

	if(MT_UNF_OTP_read(ip_license, ip_license_size, &tmp))
		return MT_FAILURE;
	*data = (otp_ip_license_type_t)tmp;
	return MT_SUCCESS;
}

MT_S32 MT_UNF_OTP_get_dram_cap(otp_dram_cap_type_t * data)
{
	MT_U32 tmp;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(data);

	if (dram_cap == -1)
		return MT_FAILURE;

	if(MT_UNF_OTP_read(dram_cap, dram_cap_size, &tmp))
		return MT_FAILURE;
	*data = (otp_dram_cap_type_t)tmp;
	return MT_SUCCESS;
}

MT_S32 MT_UNF_OTP_get_ddr_kgd(otp_ddr_kgd_type_t * data)
{
	MT_U32 tmp;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(data);

	if (ddr_kgd == -1)
		return MT_FAILURE;

	if(MT_UNF_OTP_read(ddr_kgd, ddr_kgd_size, &tmp))
		return MT_FAILURE;
	*data = (otp_ddr_kgd_type_t)tmp;
	return MT_SUCCESS;
}

MT_S32 MT_UNF_OTP_get_ctrl_authority(MT_U32 ctrl_no, MT_U32 *authority)
{
	MT_U32 bit_addr = 0;
	MT_U32 len = 0;
	MT_U32 result;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(authority);

	if (mt_chip_is_symphony1(otp_version) || otp_version == MT_CHIP_SYMPHONY3_A0)
	{
	}
	else
	{
		*authority = E_AUTH_MAX;
		return MT_SUCCESS;
	}

	if(ctrl_no >= src_max_block_num)
	{
		MT_ERR_OTP("ctrl_no is to largs \n");
		return MT_FAILURE;
	}

	if (ctrl_area_0 == -1)
		return MT_FAILURE;

	bit_addr = ctrl_area_0 + (ctrl_area_size * ctrl_no);
	len = ctrl_area_size;

	if(MT_UNF_OTP_read(bit_addr, len, &result))
	{
		return MT_FAILURE;
	}

	switch(result)
	{
		case 0x00:
			*authority = E_AUTH_RW;
			break;

		case 0x01:
			*authority = E_AUTH_RO;
			break;

		case 0x02:
			*authority = E_AUTH_WO;
			break;

		case 0x03:
			*authority = E_AUTH_FBD;
			break;

		default:
			*authority = E_AUTH_MAX;
			break;
	}

	return MT_SUCCESS;
}


MT_S32 MT_UNF_OTP_get_package_type(MT_U32 *package_type)
{
	MT_U32 result = 0;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(package_type);

	if (packet_type == -1)
		return MT_FAILURE;

	if(MT_UNF_OTP_read(packet_type, packet_type_size, &result))
	{
		return MT_FAILURE;
	}

	if (mt_chip_is_symphony1(otp_version) || otp_version == MT_CHIP_SYMPHONY3_A0)
	{
		switch(result)
		{
			case 0x00:
				*package_type = E_PACKAGE_NULL;
				break;
			case 0x01:
				*package_type = E_PACKAGE_QFP144;
				break;
			case 0x02:
				*package_type = E_PACKAGE_QFN68_SIP_512Mb_DDR2;
				break;
			case 0x03:
				*package_type = E_PACKAGE_QFN68_SIP_1Gb_DDR2;
				break;
			case 0x04:
				*package_type = E_PACKAGE_QFN68_SIP_1Gb_DDR3;
				break;
			case 0x05:
				*package_type = E_PACKAGE_BGA_SIP_512Mb_DDR2;
				break;
			case 0x06:
				*package_type = E_PACKAGE_BGA_SIP_1Gb_DDR2;
				break;
			case 0x07:
				*package_type = E_PACKAGE_BGA_SIP_1Gb_DDR3;
				break;
			case 0x12:
				*package_type = E_PACKAGE_QFN68_SIP_512Mb_DDR2_C;
				break;
			case 0x13:
				*package_type = E_PACKAGE_QFN68_SIP_1Gb_DDR2_C;
				break;
			case 0x14:
				*package_type = E_PACKAGE_QFN68_SIP_1Gb_DDR3_C;
				break;
			case 0x15:
				*package_type = E_PACKAGE_BGA_SIP_512Mb_DDR2_C;
				break;
			case 0x16:
				*package_type = E_PACKAGE_BGA_SIP_1Gb_DDR2_C;
				break;
			case 0x17:
				*package_type = E_PACKAGE_BGA_SIP_1Gb_DDR3_C;
				break;
			default:
				*package_type = E_PACKAGE_MAX;
			break;
		}
	}
	else if (mt_chip_is_symphony2(otp_version))
	{
		switch(result)
		{
			case 0:
				*package_type = E_PACKAGE_QFP144;
				break;
			case 1:
				*package_type = E_PACKAGE_QFN88;
				break;
			case 2:
				*package_type = E_PACKAGE_BGA;
				break;
			default:
				*package_type = E_PACKAGE_MAX;
				break;
		}
	}
	else if (mt_chip_is_symphony4(otp_version))
	{
		switch(result)
		{
			case 0:
				*package_type = E_PACKAGE_INFO_LQFP;
				break;
			case 1:
				*package_type = E_PACKAGE_INFO_TQFP;
				break;
			case 2:
				*package_type = E_PACKAGE_INFO_QFN;
				break;
			case 3:
				*package_type = E_PACKAGE_INFO_MQFN;
				break;
			case 4:
				*package_type = E_PACKAGE_INFO_TFBGA;
				break;
			case 5:
				*package_type = E_PACKAGE_INFO_FCCSP;
				break;
			case 6:
				*package_type = E_PACKAGE_INFO_LQFP176;
				break;
			default:
				return MT_FAILURE;
		}
	}
	else if (mt_chip_is_symphony6(otp_version))
	{
		if (MT_CHIP_SYMPHONY6_A1 == otp_version)
		{
			switch(result)
			{
				case 0:
					*package_type = E_PACKAGE_SYM6_QFP_1ST_GEN;
					break;
				case 1:
					*package_type = E_PACKAGE_SYM6_QFP_2ND_GEN;
					break;
				case 2:
					*package_type = E_PACKAGE_SYM6_QFP_3RD_GEN;
					break;
				case 4:
					*package_type = E_PACKAGE_SYM6_QFN_1ST_GEN;
					break;
				case 5:
					*package_type = E_PACKAGE_SYM6_QFN_2ND_GEN;
					break;
				case 6:
					*package_type = E_PACKAGE_SYM6_QFN_3RD_GEN;
					break;
				case 8:
					*package_type = E_PACKAGE_SYM6_BGA_1ST_GEN;
					break;
				case 9:
					*package_type = E_PACKAGE_SYM6_BGA_2ND_GEN;
					break;
				case 10:
					*package_type = E_PACKAGE_SYM6_BGA_3RD_GEN;
					break;
				default:
					*package_type = E_PACKAGE_INFO_MAX;
					break;
			}
		}
		else
		{
			switch(result)
			{
				case 2:
					*package_type = E_PACKAGE_SYM6_QFN;
					break;
				case 4:
					*package_type = E_PACKAGE_SYM6_BGA;
					break;
				case 7:
					*package_type = E_PACKAGE_SYM6_QFP;
					break;
				default:
					*package_type = E_PACKAGE_INFO_MAX;
					break;
			}
		}
	}
	else
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

/* Chip Unique ID(low 32bit), Lot Number(high 32bit) */
MT_U32 MT_UNF_OTP_get_chipid(MT_U32 *p_chipid_h, MT_U32 *p_chipid_l)
{
	MT_U32 tmp = 0;

	CHECK_DEV_OPEN_RETURN;
	CHECK_NULL_PTR_RETURN(p_chipid_h);
	CHECK_NULL_PTR_RETURN(p_chipid_l);

	if (mt_chip_is_symphony1(otp_version) || otp_version == MT_CHIP_SYMPHONY3_A0)
	{
		if(MT_UNF_OTP_read(chip_id, 32, p_chipid_l))
			return (MT_U32)MT_FAILURE;
		if(MT_UNF_OTP_read((chip_id + 32), 32, p_chipid_h))
			return (MT_U32)MT_FAILURE;
		return MT_SUCCESS;
	}
	else if (mt_chip_is_symphony2(otp_version))
	{
		if(MT_UNF_OTP_read(otp_casid,8,&tmp))
			return (MT_U32)MT_FAILURE;
		if(0xff == tmp)
		{
			if(MT_UNF_OTP_read(chip_id_ca,32,p_chipid_l) || MT_UNF_OTP_read(chip_id_ca + 32,32,p_chipid_h))
				return (MT_U32)MT_FAILURE;
			return MT_SUCCESS;
		}
		else
		{
			if(MT_UNF_OTP_read(chip_id,32,p_chipid_l) || MT_UNF_OTP_read(chip_id + 32,32,p_chipid_h))
				return (MT_U32)MT_FAILURE;
			return MT_SUCCESS;
		}
	}
	else if (mt_chip_is_symphony4(otp_version))
	{
		if(MT_UNF_OTP_read(chip_id,32,p_chipid_l) || MT_UNF_OTP_read(chip_id + 32,32,p_chipid_h))
				return (MT_U32)MT_FAILURE;
		return MT_SUCCESS;
	}
	else if (mt_chip_is_symphony6(otp_version))
	{
		if(MT_UNF_OTP_read(chip_id,32,p_chipid_l) || MT_UNF_OTP_read(chip_id + 32,32,p_chipid_h))
				return (MT_U32)MT_FAILURE;
		return MT_SUCCESS;
	}

	return (MT_U32)MT_FAILURE;
}

MT_U32 MT_UNF_OTP_get_property(MT_U32 cmd, MT_U32 *p_result)
{
	MT_U32 bit_addr = (MT_U32)-1;
	MT_U32 len = 0;
	MT_U32 result = 1;
	MT_U32 dflag = 1;

	CHECK_NULL_PTR_RETURN(p_result);

	*p_result = 0;

	CHECK_DEV_OPEN_RETURN;

	switch(cmd)
	{
		case CMD_GET_PROPERTY_HD_EN:
			bit_addr = hd_enable;
			len = hd_enable_size;
			break;

		case CMD_GET_PROPERTY_HD_DEC_EN:
			bit_addr = hd_dec_enable;
			len = hd_dec_enable_size;
			break;

		case CMD_GET_DOLBY_EN:
			bit_addr = dolby_enable;
			len = dolby_enable_size;
			break;

		case CMD_GET_DRA_EN:
			bit_addr = dra_enable;
			len = dra_enable_size;
			break;

		case CMD_GET_H265_EN:
			bit_addr = h265_enable;
			len = h265_enable_size;
			break;

		case CMD_GET_CW_CFG_EN:
			bit_addr = cw_cfg_enable;
			len = cw_cfg_enable_size;
			break;

		case CMD_GET_MACROVISION_EN:
			bit_addr = macrovision_enable;
			len = macrovision_enable_size;
			break;

		case CMD_GET_USB0_EN:
			bit_addr = usb0_enable;
			len = usb0_enable_size;
			break;

		case CMD_GET_USB1_EN:
			bit_addr = usb1_enable;
			len = usb1_enable_size;
			break;

		case CMD_GET_DVBT2_EN:
			bit_addr = dvbt2_enable;
			len = dvbt2_enable_size;
			break;

		case CMD_GET_DVBT_EN:
			bit_addr = dvbt_enable;
			len = dvbt_enable_size;
			break;

		case CMD_GET_DVBS2X_EN:
			bit_addr = dvbs2x_enable;
			len = dvbs2x_enable_size;
			break;

		case CMD_GET_DVBS_S2_S2X_EN:
			bit_addr = dvbs_s2_s2x_enable;
			len = dvbs_s2_s2x_enable_size;
			break;

		case CMD_GET_J83B_EN:
			bit_addr = j83b_enable;
			len = j83b_enable_size;
			break;

		case CMD_GET_DVBC_EN:
			bit_addr = dvbc_enable;
			len = dvbc_enable_size;
			break;

		case CMD_GET_EPHY_EN:
			bit_addr = ephy_enable;
			len = ephy_enable_size;
			break;

		case CMD_GET_HDR_EN:
			bit_addr = hdr_enable;
			len = hdr_enable_size;
			break;

		case CMD_GET_VDEC10B_EN:
			bit_addr = vdec10b_enable;
			len = vdec10b_enable_size;
			break;

		case CMD_GET_OTP_SMCD_EN:
			bit_addr = otp_smcd_enable;
			len = otp_smcd_enable_size;
			break;

		case CMD_GET_OTP_SPIMEMMAP_EN:
			bit_addr = otp_spimemmap_enable;
			len = otp_spimemmap_enable_size;
			break;

		case CMD_GET_OTP_ETHD_EN:
			bit_addr = otp_ethd_enable;
			len = otp_ethd_enable_size;
			break;

		case CMD_GET_OTP_VP9_EN:
			bit_addr = otp_vp9_enable;
			len = otp_vp9_enable_size;
			break;

		/* SYM6 */
		case CMD_GET_RV_EN:
			bit_addr = otp_rv_enable;
			len = otp_rv_enable_size;
			break;
		case CMD_GET_HDMI_EMP_EN:
			bit_addr = otp_hdmi_emp_enable;
			len = otp_hdmi_emp_enable_size;
			break;
		case CMD_GET_HDMI_EN:
			bit_addr = otp_hdmi_enable;
			len = otp_hdmi_enable_size;
			break;
		case CMD_GET_CPC_EN:
			bit_addr = otp_cpc_enable;
			len = otp_cpc_enable_size;
			break;
		case CMD_GET_ARM_CORE1_EN:
			bit_addr = otp_armcore1_enable;
			len = otp_armcore1_enable_size;
			break;
		case CMD_GET_G31_EN:
			bit_addr = otp_g31_enable;
			len = otp_g31_enable_size;
			break;
		case CMD_GET_USB1_30_EN:
			bit_addr = otp_usb1_30_enable;
			len = otp_usb1_30_enable_size;
			break;
		case CMD_GET_SL_HDR1_EN:
			bit_addr = otp_sl_hdr1_enable;
			len = otp_sl_hdr1_enable_size;
			break;
		case CMD_GET_HDR_HLG_EN:
			bit_addr = otp_hdr_hlg_enable;
			len = otp_hdr_hlg_enable_size;
			break;
		case CMD_GET_GMAC_EN:
			bit_addr = otp_gmac_enable;
			len = otp_gmac_enable_size;
			break;
		case CMD_GET_UHD_EN:
			bit_addr = otp_uhd_enable;
			len = otp_uhd_enable_size;
			break;

		case CMD_GET_DTMB_EN:
		case CMD_GET_DEMO_EN:
		case CMD_GET_DRIVER_VERSION:
		case CMD_GET_CUSTOMER_ID:
		default:
			break;
	}

	if(bit_addr == (MT_U32)-1)
	{
		MT_ERR_OTP("unknow cmd or invalid cmd\n");
		return (MT_U32)MT_FAILURE;
	}

	if(MT_UNF_OTP_read(bit_addr, len, &result))
	{
		return (MT_U32)MT_FAILURE;
	}

	if(CMD_GET_DOLBY_EN == cmd)
	{
		if (mt_chip_is_symphony1(otp_version) || otp_version == MT_CHIP_SYMPHONY3_A0)
		{
			if(MT_UNF_OTP_read(OTP_MAP_SYMPHONY_DOLBY_FORCE_DISABLE, OTP_MAP_SYMPHONY_DOLBY_FORCE_DISABLE_SIZE, &dflag))
			{
				return (MT_U32)MT_FAILURE;
			}

			if(dflag != 0)
			{
				result = 1;
			}
		}
		else if (mt_chip_is_symphony2(otp_version))
		{
			result &= 0x61;
		}
		else if (mt_chip_is_symphony4(otp_version))
		{
			if(MT_UNF_OTP_read(OTP_MAP_SYMPHONY4_DOLBY_FORCE_DISABLE, OTP_MAP_SYMPHONY4_DOLBY_FORCE_DISABLE_SIZE, &dflag))
			{
				return (MT_U32)MT_FAILURE;
			}
			if(dflag != 0)
			{
				result = 1;
			}
			else
			{
				result = ((result >> 3) & 0x1);
			}
		}
		else if (mt_chip_is_symphony6(otp_version))
		{
			if (MT_CHIP_SYMPHONY6_A1 == otp_version)
			{
				if(MT_UNF_OTP_read(OTP_MAP_SYMPHONY6_A1_DOLBY_FORCE_DISABLE, OTP_MAP_SYMPHONY6_A1_DOLBY_FORCE_DISABLE_SIZE, &dflag))
				{
					return (MT_U32)MT_FAILURE;
				}
			}
			else
			{
				if(MT_UNF_OTP_read(OTP_MAP_SYMPHONY6_DOLBY_FORCE_DISABLE, OTP_MAP_SYMPHONY6_DOLBY_FORCE_DISABLE_SIZE, &dflag))
				{
					return (MT_U32)MT_FAILURE;
				}
			}
			if(dflag != 0)
			{
				result = 1;
			}
			else
			{
				if (MT_CHIP_SYMPHONY6_A1 == otp_version)
				{
					/* bit 0-1 */
					result &= 0x3;

					/* 1/3: enable, 0/2: disable */
					if (result == 0x1 || result == 0x3)
					{
						result = 0;
					}
					else
					{
						result = 1;
					}
				}
				else
				{
					/* bit 3 */
					result = ((result >> 3) & 0x1);
				}
			}
		}
		else
		{
			result = 1;
		}
	}

	if(result == 0)
	{
		*p_result = 1;
	}
	else
	{
		*p_result = 0;
	}
	return MT_SUCCESS;
}

MT_S32 MT_UNF_OTP_lock_STBM(void)
{
	MT_S32 res = MT_SUCCESS;
	MT_U32 data = 0;

	CHECK_DEV_OPEN_RETURN;

	if (mt_chip_is_symphony1(otp_version) || otp_version == MT_CHIP_SYMPHONY3_A0)
	{
	}
	else if (mt_chip_is_symphony2(otp_version))
	{
		MT_UNF_OTP_unlock(OTP_MAP_SYMPHONY2_BLW0_ADDR,OTP_MAP_SYMPHONY2_BLW0_SIZE,OTP_MAP_SYMPHONY2_BLW0_DATA);
		res = MT_UNF_OTP_write(OTP_MAP_SYMPHONY2_BLW0_ADDR,OTP_MAP_SYMPHONY2_BLW0_SIZE,OTP_MAP_SYMPHONY2_BLW0_DATA);
		if(MT_SUCCESS != res)
			goto Return;
		res = MT_UNF_OTP_write(OTP_MAP_SYMPHONY2_SWL_BLW0_ADDR,OTP_MAP_SYMPHONY2_SWL_BLW0_SIZE,OTP_MAP_SYMPHONY2_SWL_BLW0_DATA);
	}
	else if (mt_chip_is_symphony4(otp_version))
	{
		if(MT_UNF_OTP_read(OTP_MAP_SYMPHONY4_SWL_BLW0_ADDR, OTP_MAP_SYMPHONY4_SWL_BLW0_SIZE, &data))
		{
			return MT_FAILURE;
		}
		if(data != 1)
		{
			MT_ERR_OTP("data = %x\n",data);
			return MT_FAILURE;
		}
		MT_UNF_OTP_unlock(0,32,0);
		res = MT_UNF_OTP_write(OTP_MAP_SYMPHONY4_BLW0_ADDR,OTP_MAP_SYMPHONY4_BLW0_SIZE,OTP_MAP_SYMPHONY4_BLW0_DATA);
	}
	else if (mt_chip_is_symphony6(otp_version))
	{
		/* Sym6 not support MT_UNF_OTP_lock_STBM */
	}

Return:
	return res;
}

