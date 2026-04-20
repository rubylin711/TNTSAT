/*
 * Copyright (C) 2020 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 *
 * This program is confidential and proprietary to Montage Technology Group Limited
 * and its affiliated companies(Montage), and may not be copied, reproduced, modified,
 * disclosed to others, published or used, in whole or in part, without the express
 * prior written permission of Montage.
 */
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <string.h>
#include <pthread.h>

#include "mt_type.h"
#include "ca_types.h"
#include "nocs_csd.h"
#include "mt_unf_descrambler.h"
#include "nocs_csd_impl.h"
#include "mt_unf_misc.h"
#include "mt_unf_cipher_v2.h"
#include "mt_common.h"
#include "mt_unf_flash.h"
#include "mt_sec_ext.h"

//#define CSD_DEBUG
#ifdef CSD_DEBUG
#define CSD_LOG(level, fmt, ...) \
	({ \
		if (level <= csd_log_level) \
		printf("[%s:%u]"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
	})
#define CSD_ERR(fmt,  ...)	CSD_LOG(CSD_LOG_ERROR, fmt, ##__VA_ARGS__)
#define CSD_DBG(fmt,  ...) 	CSD_LOG(CSD_LOG_DEBUG, fmt, ##__VA_ARGS__)
#define CSD_INFO(fmt, ...)	CSD_LOG(CSD_LOG_INFO, fmt, ##__VA_ARGS__)
#define CSD_DUMP(str, addr, size) csd_dump(str, addr, size)
#else
#define CSD_ERR(fmt,  ...)  \
	do {            \
	} while (0)
#define CSD_DBG(fmt,  ...)  \
	do {            \
	} while (0)
#define	CSD_INFO(fmt, ...)  \
	do {            \
	} while (0)
#define CSD_DUMP(fmt, ...)    \
	do {            \
	} while (0)
#endif

#define RETURN_IF_FAIL(COND) \
	({ \
		if (!(COND)) \
		{ \
		 CSD_ERR("Check '%s' failed.", #COND); \
		 return; \
		} \
	})

#define RETURN_VAL_IF_FAIL(COND, VAL) \
	({ \
		if (!(COND)) \
		{ \
			CSD_ERR("Check '%s' failed.\n", #COND); \
			return (VAL); \
		} \
	})

#define ARRAY_SIZE(ARRAY)	(sizeof(ARRAY) / sizeof(ARRAY[0]))

enum CSD_LOG_LEVEL {
	CSD_LOG_NONE = 0,
	CSD_LOG_ERROR,
	CSD_LOG_DEBUG,
	CSD_LOG_INFO,
};

/*
 * NOCS standard predefined PV IDs
 */

#define CSD_NOCS_PVID_BOOTMODE_ID                      0x4000
    /**< Bootmode PV type. */

#define CSD_NOCS_PVID_DIF_ID                           0x4001
    /**< Debug interface PV type. */

#define CSD_NOCS_PVID_KLD_DEACTIVATION_ID              0x4002
    /**< Key Ladder Deactivation PV type. */

#define CSD_NOCS_PV_FLASH_ACTIVATION_ID                0x4003
    /**< Flash Protection Activation PV type. */

#define CSD_NOCS_PVID_SCS_ACTIVATION_ID                0x4004
    /**< SCS activation PV type. */

#define CSD_NOCS_PVID_USS_ID                           0x4005
    /**< Unscrambled Storage Size PV type. */

#define CSD_NOCS_PVID_MSID_ID                          0x4006
    /**< Market Segment Id. */

#define CSD_NOCS_PVID_MSIDL_ID                         0x4007
    /**< Lock of the Market Segment Id. */

#define CSD_NOCS_PVID_STBCASN_ID                       0x4008
    /**< STB_CA_SN. */

#define CSD_NOCS_PVID_STBCASNL_ID                      0x4009
    /**< STB_CA_SN Lock. */

#define CSD_NOCS_PVID_VER_ID                           0x400A
    /**< Versioning_REF. */

#define CSD_NOCS_PVID_EBFDP_ID                         0x400B
    /**< External Boot Flash Device Parameters (optional). */

#define CSD_NOCS_PVID_ERP_ID                           0x400C
    /**< External RAM Parameters (optional). */

#define CSD_NOCS_PVID_SCS_DTE_Boot_Code_Area_ID        0x400D
    /**< SCS_DTE_Boot_Code_area adress (optional). */

#define CSD_NOCS_PVID_AI_ID                            0x400E
    /**< Active Indicator (optional). */

#define CSD_NOCS_PVID_SCSTASize_ID                     0x400F
    /**< SCS_Total_Area size (optional). */

#define CSD_NOCS_PVID_ESCSFWUA_ID                      0x4010
    /**< Full Ext_SCS_FW upgrade address (optional). */

#define CSD_NOCS_PVID_PESCSFWUA_ID                     0x4011
    /**< Patch Ext_SCS_FW upgrade address (optional). */

#define CSD_NOCS_PVID_ESCSFWUS_ID                      0x4012
    /**< Full Ext_SCS_FW upgrade size (optional). */

#define CSD_NOCS_PVID_PESCSFWUS_ID                     0x4013
    /**< Patch Ext_SCS_FW upgrade size (optional). */

#define CSD_NOCS_PVID_RAM_SCRAMBLE_ACTIVATION_ID       0x4014
    /**< RAM scrambling activation PV type. */

#define CSD_NOCS_PVID_TEE_PRIVILEGE_ACTIVATION_ID      0x4020
    /**< TEE privilege mode activation PV type. */

static MT_U32 csd_log_level = CSD_LOG_NONE;

static void csd_set_log_level(MT_U32 level)
{
	csd_log_level = level;
}

#ifdef CSD_DEBUG
static void csd_dump(char *str, MT_U8 * addr, MT_U32 size)
{
#if 1
	MT_U32 i;

	printf("%s: %p(%d)\n", str, addr, size);
	for (i = 0; i < size; i++) {
		printf("%02x ", addr[i]);
		if ((i % 16) == 15)
			printf("\n");
	}
	printf("\n");
#endif
}
#endif

/* Nagra Root Key List */
#define NOCS_AES_ROOTKEY							MT_CIPHER_KEYLADDER_SCK_0
#define NOCS_TDES_ROOTKEY							MT_CIPHER_KEYLADDER_SCK_1
#define NOCS_CSA2_ROOTKEY							MT_CIPHER_KEYLADDER_SCK_2
#define NOCS_CSA3_ROOTKEY							MT_CIPHER_KEYLADDER_SCK_3
#define NOCS_FLASH_PROTECT_ROOTKEY				MT_CIPHER_KEYLADDER_SCK_13
#define NOCS_ETSI_KDF_ROOTKEY						MT_CIPHER_KEYLADDER_SCK_15
#define NOCS_ETSI_CW_ROOTKEY						MT_CIPHER_KEYLADDER_PRIVATE_0
#define NOCS_ETSI_NONCE_ROOTKEY					MT_CIPHER_KEYLADDER_PRIVATE_1

#define NOCS_EMI_MPEG_TS_DVB_CSA2						0x0000
#define NOCS_EMI_MPEG_TS_DVB_CSA3						0x0001
#define NOCS_EMI_MPEG_TS_DVB_ASA_64						0x0010
#define NOCS_EMI_MPEG_TS_DVB_ASA_128					0x0011
#define NOCS_EMI_MPEG_TS_DVB_ASA_LIGHT					0x0012
#define NOCS_EMI_MPEG_TS_DVB_AES128_IDSA				0x0020
#define NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR		0x0021
#define NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR		0x0022
#define NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1			0x0023
#define NOCS_EMI_MPEG_TS_DVB_AES128_ECB_HEAD_CLEAR              0x0024
#define NOCS_EMI_MPEG_TS_DVB_TDES_CBC_ZEROIV_DVS042	0x0040
#define NOCS_EMI_MPEG_TS_DVB_TDES_ECB_TAIL_CLEAR		0x0041
#define NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR			0x4020
#define NOCS_EMI_AES128_ECB_TAIL_CLEAR					0x4021
#define NOCS_EMI_AES128_MPEG_DASH_CBC_CLEAR_PADDING	0x4022
#define NOCS_EMI_AES128_CBC_PKCS7_PADDING				0x4023
#define NOCS_EMI_AES128_MPEG_DASH_CTR					0x4024
#define NOCS_EMI_AES128_CBC_TAIL_CLEAR					0x4026
#define NOCS_EMI_AES128_CTR								0x4027
#define NOCS_EMI_AES128_HLS								0x4029
#define NOCS_EMI_AES128_MPEG_DASH_CBCS                                0x402A
#define NOCS_EMI_AES128_MPEG_DASH_CTR_CENS                       0x402B
#define NOCS_EMI_TDES_CBC_ZEROIV_TAIL_CLEAR				0x4040
#define NOCS_EMI_TDES_ECB_TAIL_CLEAR						0x4041
#define NOCS_EMI_TDES_CBC_TAIL_CLEAR						0x4043

#define OTP_NUIDL_OFFSET				(0xfab << 2)
#define OTP_NUIDL_SIZE					(4)
#define OTP_NUIDL_SHIFT				(0)
#define OTP_NUIDL_BITS					(32)

#define OTP_NUIDH_OFFSET				(0xfac << 2)
#define OTP_NUIDH_SIZE					(4)
#define OTP_NUIDH_SHIFT				(0)
#define OTP_NUIDH_BITS					(32)

#define OTP_CHK_NUM_0_OFFSET			(0x1080 << 2)
#define OTP_CHK_NUM_0_SIZE			(4)
#define OTP_CHK_NUM_0_SHIFT			(0)
#define OTP_CHK_NUM_0_BITS			(32)

#define OTP_DEACTIVE_SCK_OFFSET		(0xf50 << 2)
#define OTP_DEACTIVE_SCK_SIZE			(1)
#define OTP_DEACTIVE_SCK_SHIFT		(0)
#define OTP_DEACTIVE_SCK_BITS			(4)
#define OTP_DEACTIVE_SCK_VALUE		(0x0f << OTP_DEACTIVE_SCK_SHIFT)

#define OTP_STB_CA_SN_OFFSET			(0xf9d << 2)
#define OTP_STB_CA_SN_SIZE				(4)
#define OTP_STB_CA_SN_SHIFT			(0)
#define OTP_STB_CA_SN_BITS				(32)

#define OTP_SWL_STB_CA_SN_OFFSET		(0xf68 << 2)
#define OTP_SWL_STB_CA_SN_SIZE		(1)
#define OTP_SWL_STB_CA_SN_SHIFT		(2)
#define OTP_SWL_STB_CA_SN_BITS		(3)
#define OTP_UNLOCK_STB_CA_SN_VALUE	(0x1 << OTP_SWL_STB_CA_SN_SHIFT)
#define OTP_LOCK_STB_CA_SN_VALUE		(0x7 << OTP_SWL_STB_CA_SN_SHIFT)

#define OTP_MSID0_OFFSET				(0xe7d << 2)
#define OTP_MSID0_SIZE					(4)
#define OTP_MSID0_SHIFT				(0)
#define OTP_MSID0_BITS					(32)

#define OTP_SWL_MSID0_OFFSET			(0xf6a << 2)
#define OTP_SWL_MSID0_SIZE				(1)
#define OTP_SWL_MSID0_SHIFT			(2)
#define OTP_SWL_MSID0_BITS				(3)
#define OTP_UNLOCK_MSID0_VALUE		(0x1 << OTP_SWL_MSID0_SHIFT)
#define OTP_LOCK_MSID0_VALUE			(0x7 << OTP_SWL_MSID0_SHIFT)

#define OTP_SECURE_EN_OFFSET			((0xe8f << 2))
#define OTP_SECURE_EN_SIZE				(1)
#define OTP_SECURE_EN_SHIFT			(4)
#define OTP_SECURE_EN_BITS				(4)

#define OTP_AKL_REEKILLED_OFFSET		((0xf9e << 2) + 3)
#define OTP_AKL_REEKILLED_SIZE			(1)
#define OTP_AKL_REEKILLED_SHIFT		(4)
#define OTP_AKL_REEKILLED_BITS			(4)

#define OTP_VER_NV_REF0_OFFSET		(0xe7e << 2)
#define OTP_VER_NV_REF0_SIZE			(4)
#define OTP_VER_NV_REF0_SHIFT			(0)
#define OTP_VER_NV_REF0_BITS			(32)

#define ARRAY_SIZE(ARRAY)	(sizeof(ARRAY) / sizeof(ARRAY[0]))
#define MASK(BITS)	((MT_U32)(1LL << BITS) - 1)

#define OTP_OFFSET(NAME)	(OTP_##NAME##_OFFSET)
#define OTP_SIZE(NAME)	(OTP_##NAME##_SIZE)
#define OTP_SHIFT(NAME)	(OTP_##NAME##_SHIFT)
#define OTP_BITS(NAME)	(OTP_##NAME##_BITS)
#define OTP_MASK(NAME)	(MASK(OTP_BITS(NAME)) << OTP_SHIFT(NAME))
#define OTP_VAL(NAME) \
	OTP_OFFSET(NAME), OTP_SIZE(NAME), OTP_MASK(NAME)

#define FLASH_ADDR(NAME)	(NAME##_ADDR)
#define FLASH_SIZE(NAME)	(NAME##_SIZE)
#define FLASH_VAL(NAME) \
	FLASH_ADDR(NAME), FLASH_SIZE(NAME)

#define BOOTINIT_PARTITION_NAME		"btinit.img"

#define RAM_SCRAMBLE_ACTIVATION_ADDR	(0x204)
#define RAM_SCRAMBLE_ACTIVATION_SIZE		(0x4)
#define RAM_SCRAMBLE_ACTIVATION_ENABLED	(0xaaaa5555)
#define RAM_SCRAMBLE_ACTIVATION_DISABLED	(0x0)

#define FLASH_PROTECT_ACTIVATION_ADDR	(0x218)
#define FLASH_PROTECT_ACTIVATION_SIZE	(0x4)
#define FLASH_PROTECTION_ACTIVATION_ENABLED	(0xaaaa5555)
#define FLASH_PROTECTION_ACTIVATION_DISABLED	(0x0)

#define SCS_DTE_BOOT_CODE_AREA_ADDR_ADDR	(0x210)
#define SCS_DTE_BOOT_CODE_AREA_ADDR_SIZE	(0x4)

#define SCS_TOTAL_AREA_SIZE_ADDR	(0x214)
#define SCS_TOTAL_AREA_SIZE_SIZE	(0x4)

#define REG_BOOT_CFG		(0xBf140020)	//bit0-2:000spi-nor, 100-spi-nand, 111-pnand

#define OTP_BOOT_MODE_SEL_OFFSET		(0xfad << 2)
#define OTP_BOOT_MODE_SEL_SIZE		(1)
#define OTP_BOOT_MODE_SEL_SHIFT		(4)
#define OTP_BOOT_MODE_SEL_BITS		(3)
#define BOOT_MODE_SEL_STRAP_PIN		(0x00)
#define BOOT_MODE_SEL_SPI_NOR			(0x30)
#define BOOT_MODE_SEL_PNAND			(0x50)
#define BOOT_MODE_SEL_SPI_NAND		(0x60)
#define BOOT_MODE_SEL_EMMC		(0x70)
#define BOOT_MODE_SEL_IS_STRAP_PIN(MODE) ({(MODE & OTP_MASK(BOOT_MODE_SEL)) == BOOT_MODE_SEL_STRAP_PIN;})
#define BOOT_MODE_SEL_IS_SPI_NOR(MODE)	({((MODE & OTP_MASK(BOOT_MODE_SEL)) == BOOT_MODE_SEL_SPI_NOR);})
#define BOOT_MODE_SEL_IS_PNAND(MODE)		({(MODE & OTP_MASK(BOOT_MODE_SEL)) == BOOT_MODE_SEL_PNAND;})
#define BOOT_MODE_SEL_IS_SPI_NAND(MODE)	({(MODE & OTP_MASK(BOOT_MODE_SEL)) == BOOT_MODE_SEL_SPI_NAND;})
#define BOOT_MODE_SEL_IS_EMMC(MODE)	({(MODE & OTP_MASK(BOOT_MODE_SEL)) == BOOT_MODE_SEL_EMMC;})

#define OTP_REEJTAG_MODE_OFFSET		((0xfbe << 2) + 2)
#define OTP_REEJTAG_MODE_SIZE			(2)
#define OTP_REEJTAG_MODE_SHIFT		(2)
#define OTP_REEJTAG_MODE_BITS			(9)
#define REEJTAG_MODE_OPEN				(0x0 << OTP_REEJTAG_MODE_SHIFT)
#define REEJTAG_MODE_PWD				(0x3f << OTP_REEJTAG_MODE_SHIFT)
#define REEJTAG_MODE_CLOSED			(0x1ff << OTP_REEJTAG_MODE_SHIFT)

#define OTP_I2C_MODE_OFFSET		(0xfbe << 2)
#define OTP_I2C_MODE_SIZE			(2)
#define OTP_I2C_MODE_SHIFT			(0)
#define OTP_I2C_MODE_BITS			(9)
#define I2C_MODE_OPEN		(0x0)
#define I2C_MODE_PWD		(0x3f)
#define I2C_MODE_CLOSED	(0x1ff)
#define I2C_MODE_IS_OPEN(MODE)		({(MODE & OTP_MASK(I2C_MODE)) == I2C_MODE_OPEN;})
#define I2C_MODE_IS_PWD(MODE)		({(MODE & I2C_MODE_PWD) == I2C_MODE_PWD;})
#define I2C_MODE_IS_CLOSED(MODE)	({(MODE & I2C_MODE_CLOSED) == I2C_MODE_CLOSED;})

typedef TCsdUnsignedInt32 TCsdPvId;

struct {
	TCsdScsPvId csd_id;
	TCsdUnsignedInt32 ctd_id;
} csd_pv_map[] = {
	{
	CSD_SCS_PV_EXT_BOOT_FLASH_DEV_PARAMS, CSD_NOCS_PVID_EBFDP_ID}, {
	CSD_SCS_PV_EXT_RAM_PARAMS, CSD_NOCS_PVID_ERP_ID}, {
	CSD_SCS_PV_SCS_TOTAL_AREA_SIZE, CSD_NOCS_PVID_SCSTASize_ID}, {
	CSD_SCS_PV_ACTIVE_INDICATOR, CSD_NOCS_PVID_AI_ID}, {
	CSD_SCS_PV_FULL_EXT_FW_UPGRADE_ADDR, CSD_NOCS_PVID_ESCSFWUA_ID}, {
	CSD_SCS_PV_FULL_EXT_FW_UPGRADE_SIZE, CSD_NOCS_PVID_ESCSFWUS_ID}, {
	CSD_SCS_PV_PATCH_EXT_FW_UPGRADE_ADDR, CSD_NOCS_PVID_PESCSFWUA_ID}, {
	CSD_SCS_PV_PATCH_EXT_FW_UPGRADE_SIZE, CSD_NOCS_PVID_PESCSFWUS_ID}, {
CSD_SCS_PV_VERSIONING_REF, CSD_NOCS_PVID_VER_ID},};

static struct {
	TCsdPvId pv_id;
	MT_U32 otp_offset;
	MT_U32 otp_size;
	MT_U32 otp_mask;
	MT_BOOL reverse;
} pv_otp_map[] = {
	{
	CSD_NOCS_PVID_BOOTMODE_ID, OTP_VAL(BOOT_MODE_SEL), MT_FALSE}, {
	CSD_NOCS_PVID_KLD_DEACTIVATION_ID, OTP_VAL(AKL_REEKILLED), MT_FALSE},
	{
	CSD_NOCS_PVID_SCS_ACTIVATION_ID, OTP_VAL(SECURE_EN), MT_FALSE}, {
	CSD_NOCS_PVID_MSID_ID, OTP_VAL(MSID0), MT_TRUE}, {
	CSD_NOCS_PVID_MSIDL_ID, OTP_VAL(SWL_MSID0), MT_FALSE}, {
	CSD_NOCS_PVID_STBCASN_ID, OTP_VAL(STB_CA_SN), MT_TRUE}, {
	CSD_NOCS_PVID_STBCASNL_ID, OTP_VAL(SWL_STB_CA_SN), MT_FALSE}, {
	CSD_NOCS_PVID_VER_ID, OTP_VAL(VER_NV_REF0), MT_TRUE},};

static struct {
	TCsdPvId pv_id;
	MT_U32 pv_size;
	MT_U32 pv_value;
} pv_const_map[] = {
	{
	CSD_NOCS_PVID_DIF_ID, 4, 0}, {
	CSD_NOCS_PVID_USS_ID, 4, 0},};

typedef enum CHIP_REVISION_E {
	CHIP_REVISION_A0 = 0xB000,
	CHIP_REVISION_A1,
	CHIP_REVISION_A2,
	CHIP_REVISION_A3,
	CHIP_REVISION_A4,
	CHIP_REVISION_A5,
	CHIP_REVISION_A6,
	CHIP_REVISION_A7,
	CHIP_REVISION_A8,
	CHIP_REVISION_A9,
	CHIP_REVISION_MAX,
} CHIP_REVISION;
typedef struct CHIP_REVISION_PAIR_S {
	CHIP_REVISION data;
	char chars[4];
} CHIP_REVISION_PAIR;

typedef enum CIPHER_KEY_MODE_E {
	CIPHER_CLEAR_KEY,
	CIPHER_SECRET_KEY,
	CIPHER_KEY_MAX,
} CIPHER_KEY_MODE;

#define DEFAULT_MTDDEV		(8)

static TUnsignedInt8 kl_prot_keys[2][16] = {
	{
		0xa9, 0x32, 0x30, 0x31, 0x31, 0x4e, 0x61, 0x67,
		0x72, 0x61, 0x76, 0x69, 0x73, 0x69, 0x6f, 0x6e,},
	{
	0xa9, 0x32, 0x30, 0x31, 0x31, 0x4e, 0x61, 0x67,
	0x72, 0x61, 0x76, 0x69, 0x73, 0x69, 0x6f, 0x6e}
};


static MT_BOOL csd_inited = MT_FALSE;
/*
static pthread_mutex_t g_otp_op_mutex = PTHREAD_MUTEX_INITIALIZER;
#define otp_lock() (void) pthread_mutex_lock(&g_otp_op_mutex);
#define otp_unlock() (void) pthread_mutex_unlock(&g_otp_op_mutex);
*/

static TCsdStatus pcsdByteSwap(TUnsignedInt8 * pxBuffer, TUnsignedInt32 xLen)
{
	TUnsignedInt32 index = 0;
	TUnsignedInt8 tmp = 0;
	for (index = 0; index < xLen / 2; index++) {
		tmp = pxBuffer[index];
		pxBuffer[index] = pxBuffer[xLen - 1 - index];
		pxBuffer[xLen - 1 - index] = tmp;
	}
	return CSD_NO_ERROR;
}

static mt_u32 crc32_calc(mt_u8 * data, mt_s32 len, mt_u32 crc)
{
	static const mt_u32 table[256] = {
		0x00000000U, 0x77073096U, 0xEE0E612CU, 0x990951BAU,
		0x076DC419U, 0x706AF48FU, 0xE963A535U, 0x9E6495A3U,
		0x0EDB8832U, 0x79DCB8A4U, 0xE0D5E91EU, 0x97D2D988U,
		0x09B64C2BU, 0x7EB17CBDU, 0xE7B82D07U, 0x90BF1D91U,
		0x1DB71064U, 0x6AB020F2U, 0xF3B97148U, 0x84BE41DEU,
		0x1ADAD47DU, 0x6DDDE4EBU, 0xF4D4B551U, 0x83D385C7U,
		0x136C9856U, 0x646BA8C0U, 0xFD62F97AU, 0x8A65C9ECU,
		0x14015C4FU, 0x63066CD9U, 0xFA0F3D63U, 0x8D080DF5U,
		0x3B6E20C8U, 0x4C69105EU, 0xD56041E4U, 0xA2677172U,
		0x3C03E4D1U, 0x4B04D447U, 0xD20D85FDU, 0xA50AB56BU,
		0x35B5A8FAU, 0x42B2986CU, 0xDBBBC9D6U, 0xACBCF940U,
		0x32D86CE3U, 0x45DF5C75U, 0xDCD60DCFU, 0xABD13D59U,
		0x26D930ACU, 0x51DE003AU, 0xC8D75180U, 0xBFD06116U,
		0x21B4F4B5U, 0x56B3C423U, 0xCFBA9599U, 0xB8BDA50FU,
		0x2802B89EU, 0x5F058808U, 0xC60CD9B2U, 0xB10BE924U,
		0x2F6F7C87U, 0x58684C11U, 0xC1611DABU, 0xB6662D3DU,
		0x76DC4190U, 0x01DB7106U, 0x98D220BCU, 0xEFD5102AU,
		0x71B18589U, 0x06B6B51FU, 0x9FBFE4A5U, 0xE8B8D433U,
		0x7807C9A2U, 0x0F00F934U, 0x9609A88EU, 0xE10E9818U,
		0x7F6A0DBBU, 0x086D3D2DU, 0x91646C97U, 0xE6635C01U,
		0x6B6B51F4U, 0x1C6C6162U, 0x856530D8U, 0xF262004EU,
		0x6C0695EDU, 0x1B01A57BU, 0x8208F4C1U, 0xF50FC457U,
		0x65B0D9C6U, 0x12B7E950U, 0x8BBEB8EAU, 0xFCB9887CU,
		0x62DD1DDFU, 0x15DA2D49U, 0x8CD37CF3U, 0xFBD44C65U,
		0x4DB26158U, 0x3AB551CEU, 0xA3BC0074U, 0xD4BB30E2U,
		0x4ADFA541U, 0x3DD895D7U, 0xA4D1C46DU, 0xD3D6F4FBU,
		0x4369E96AU, 0x346ED9FCU, 0xAD678846U, 0xDA60B8D0U,
		0x44042D73U, 0x33031DE5U, 0xAA0A4C5FU, 0xDD0D7CC9U,
		0x5005713CU, 0x270241AAU, 0xBE0B1010U, 0xC90C2086U,
		0x5768B525U, 0x206F85B3U, 0xB966D409U, 0xCE61E49FU,
		0x5EDEF90EU, 0x29D9C998U, 0xB0D09822U, 0xC7D7A8B4U,
		0x59B33D17U, 0x2EB40D81U, 0xB7BD5C3BU, 0xC0BA6CADU,
		0xEDB88320U, 0x9ABFB3B6U, 0x03B6E20CU, 0x74B1D29AU,
		0xEAD54739U, 0x9DD277AFU, 0x04DB2615U, 0x73DC1683U,
		0xE3630B12U, 0x94643B84U, 0x0D6D6A3EU, 0x7A6A5AA8U,
		0xE40ECF0BU, 0x9309FF9DU, 0x0A00AE27U, 0x7D079EB1U,
		0xF00F9344U, 0x8708A3D2U, 0x1E01F268U, 0x6906C2FEU,
		0xF762575DU, 0x806567CBU, 0x196C3671U, 0x6E6B06E7U,
		0xFED41B76U, 0x89D32BE0U, 0x10DA7A5AU, 0x67DD4ACCU,
		0xF9B9DF6FU, 0x8EBEEFF9U, 0x17B7BE43U, 0x60B08ED5U,
		0xD6D6A3E8U, 0xA1D1937EU, 0x38D8C2C4U, 0x4FDFF252U,
		0xD1BB67F1U, 0xA6BC5767U, 0x3FB506DDU, 0x48B2364BU,
		0xD80D2BDAU, 0xAF0A1B4CU, 0x36034AF6U, 0x41047A60U,
		0xDF60EFC3U, 0xA867DF55U, 0x316E8EEFU, 0x4669BE79U,
		0xCB61B38CU, 0xBC66831AU, 0x256FD2A0U, 0x5268E236U,
		0xCC0C7795U, 0xBB0B4703U, 0x220216B9U, 0x5505262FU,
		0xC5BA3BBEU, 0xB2BD0B28U, 0x2BB45A92U, 0x5CB36A04U,
		0xC2D7FFA7U, 0xB5D0CF31U, 0x2CD99E8BU, 0x5BDEAE1DU,
		0x9B64C2B0U, 0xEC63F226U, 0x756AA39CU, 0x026D930AU,
		0x9C0906A9U, 0xEB0E363FU, 0x72076785U, 0x05005713U,
		0x95BF4A82U, 0xE2B87A14U, 0x7BB12BAEU, 0x0CB61B38U,
		0x92D28E9BU, 0xE5D5BE0DU, 0x7CDCEFB7U, 0x0BDBDF21U,
		0x86D3D2D4U, 0xF1D4E242U, 0x68DDB3F8U, 0x1FDA836EU,
		0x81BE16CDU, 0xF6B9265BU, 0x6FB077E1U, 0x18B74777U,
		0x88085AE6U, 0xFF0F6A70U, 0x66063BCAU, 0x11010B5CU,
		0x8F659EFFU, 0xF862AE69U, 0x616BFFD3U, 0x166CCF45U,
		0xA00AE278U, 0xD70DD2EEU, 0x4E048354U, 0x3903B3C2U,
		0xA7672661U, 0xD06016F7U, 0x4969474DU, 0x3E6E77DBU,
		0xAED16A4AU, 0xD9D65ADCU, 0x40DF0B66U, 0x37D83BF0U,
		0xA9BCAE53U, 0xDEBB9EC5U, 0x47B2CF7FU, 0x30B5FFE9U,
		0xBDBDF21CU, 0xCABAC28AU, 0x53B39330U, 0x24B4A3A6U,
		0xBAD03605U, 0xCDD70693U, 0x54DE5729U, 0x23D967BFU,
		0xB3667A2EU, 0xC4614AB8U, 0x5D681B02U, 0x2A6F2B94U,
		0xB40BBE37U, 0xC30C8EA1U, 0x5A05DF1BU, 0x2D02EF8DU,
	};

	crc = crc ^ 0xFFFFFFFFU;
	while (len > 0) {
		crc = table[*data ^ (mt_u8) crc] ^ (crc >> 8);
		data++;
		len--;
	}
	crc = crc ^ 0xFFFFFFFFU;
	return crc;
}

static TBoolean pcsdChkEmiRange(TUnsignedInt16 xEMI)
{
	switch (xEMI) {
	case NOCS_EMI_MPEG_TS_DVB_CSA2:
	case NOCS_EMI_MPEG_TS_DVB_CSA3:
	case NOCS_EMI_MPEG_TS_DVB_ASA_64:
	case NOCS_EMI_MPEG_TS_DVB_ASA_128:
	case NOCS_EMI_MPEG_TS_DVB_ASA_LIGHT:
	case NOCS_EMI_MPEG_TS_DVB_AES128_IDSA:
	case NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR:
	case NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR:
	case NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1:
	case NOCS_EMI_MPEG_TS_DVB_TDES_CBC_ZEROIV_DVS042:
	case NOCS_EMI_MPEG_TS_DVB_TDES_ECB_TAIL_CLEAR:
	case NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR:
	case NOCS_EMI_AES128_ECB_TAIL_CLEAR:
	case NOCS_EMI_AES128_MPEG_DASH_CBC_CLEAR_PADDING:
	case NOCS_EMI_AES128_CBC_PKCS7_PADDING:
	case NOCS_EMI_AES128_MPEG_DASH_CTR:
	case NOCS_EMI_AES128_CBC_TAIL_CLEAR:
	case NOCS_EMI_AES128_CTR:
	case NOCS_EMI_AES128_HLS:
	case NOCS_EMI_AES128_MPEG_DASH_CBCS:
	case NOCS_EMI_AES128_MPEG_DASH_CTR_CENS:
	case NOCS_EMI_TDES_CBC_ZEROIV_TAIL_CLEAR:
	case NOCS_EMI_TDES_ECB_TAIL_CLEAR:
	case NOCS_EMI_TDES_CBC_TAIL_CLEAR:
		return TRUE;
	default:
		return FALSE;
	}
}

static TBoolean pcsdChkKeySize(TUnsignedInt16 xEMI, size_t xClearTextKeySize)
{
	switch (xEMI) {
	case NOCS_EMI_MPEG_TS_DVB_CSA2:
		if (8 == xClearTextKeySize) {
			return TRUE;
		} else {
			return FALSE;
		}
	case NOCS_EMI_MPEG_TS_DVB_CSA3:
	case NOCS_EMI_MPEG_TS_DVB_AES128_IDSA:
	case NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR:
	case NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR:
	case NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1:
	case NOCS_EMI_MPEG_TS_DVB_TDES_CBC_ZEROIV_DVS042:
	case NOCS_EMI_MPEG_TS_DVB_TDES_ECB_TAIL_CLEAR:
	case NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR:
	case NOCS_EMI_AES128_ECB_TAIL_CLEAR:
	case NOCS_EMI_AES128_MPEG_DASH_CBC_CLEAR_PADDING:
	case NOCS_EMI_AES128_CBC_PKCS7_PADDING:
	case NOCS_EMI_AES128_MPEG_DASH_CTR:
	case NOCS_EMI_AES128_CBC_TAIL_CLEAR:
	case NOCS_EMI_AES128_CTR:
	case NOCS_EMI_AES128_HLS:
	case NOCS_EMI_AES128_MPEG_DASH_CBCS:
	case NOCS_EMI_AES128_MPEG_DASH_CTR_CENS:
	case NOCS_EMI_TDES_CBC_ZEROIV_TAIL_CLEAR:
	case NOCS_EMI_TDES_ECB_TAIL_CLEAR:
	case NOCS_EMI_TDES_CBC_TAIL_CLEAR:
		if (16 == xClearTextKeySize) {
			return TRUE;
		} else {
			return FALSE;
		}
	default:
		return FALSE;
	}
}

static TBoolean pcsdChkIVSize(TUnsignedInt16 xEMI, size_t xInitVectorSize)
{

	switch (xEMI) {
	case NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR:
	case NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1:
	case NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR:
	case NOCS_EMI_AES128_MPEG_DASH_CBC_CLEAR_PADDING:
	case NOCS_EMI_AES128_CBC_PKCS7_PADDING:
	case NOCS_EMI_AES128_MPEG_DASH_CTR:
	case NOCS_EMI_AES128_CBC_TAIL_CLEAR:
	case NOCS_EMI_AES128_CTR:
	case NOCS_EMI_AES128_HLS:
	case NOCS_EMI_AES128_MPEG_DASH_CBCS:
	case NOCS_EMI_AES128_MPEG_DASH_CTR_CENS:
		if (16 == xInitVectorSize) {
			return TRUE;
		} else {
			return FALSE;
		}
	case NOCS_EMI_MPEG_TS_DVB_TDES_CBC_ZEROIV_DVS042:
	case NOCS_EMI_TDES_CBC_ZEROIV_TAIL_CLEAR:
	case NOCS_EMI_TDES_CBC_TAIL_CLEAR:
		if (8 == xInitVectorSize) {
			return TRUE;
		} else {
			return FALSE;
		}
	case NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR:
	case NOCS_EMI_MPEG_TS_DVB_TDES_ECB_TAIL_CLEAR:
	case NOCS_EMI_AES128_ECB_TAIL_CLEAR:
	case NOCS_EMI_TDES_ECB_TAIL_CLEAR:
		if (0 == xInitVectorSize) {
			return TRUE;
		} else {
			return FALSE;
		}
	default:
		return FALSE;
	}
}

static MT_CIPHER_KEYLADDER_SOURCE_E pcsdGetRootKeyID(TUnsignedInt16 xEmi)
{
	MT_CIPHER_KEYLADDER_SOURCE_E rootkey = MT_CIPHER_KEYLADDER_SCK_UNKNOWN;

	if (xEmi == NOCS_EMI_MPEG_TS_DVB_CSA2) {
		rootkey = NOCS_CSA2_ROOTKEY;
	} else if (xEmi == NOCS_EMI_MPEG_TS_DVB_CSA3) {
		rootkey = NOCS_CSA3_ROOTKEY;
	} else if ((xEmi & 0x00F0) == 0x0020) {
		rootkey = NOCS_AES_ROOTKEY;
	} else if ((xEmi & 0x00F0) == 0x0040) {
		rootkey = NOCS_TDES_ROOTKEY;
	} else {
		CSD_ERR("no rootkey for EMI %04x\n", xEmi);
		return MT_CIPHER_KEYLADDER_SCK_UNKNOWN;
	}
	return rootkey;
}

static TBoolean pcsdSession2KeyCtrl(TUnsignedInt8 isEncryptionSession,
	TUnsignedInt16 xEMI,
	MT_CIPHER_CTRL_S * pCtrl)
{
	if (isEncryptionSession) {
		pCtrl->operation = MT_CIPHER_OPERATION_ENCRYPT;
	} else {
		pCtrl->operation = MT_CIPHER_OPERATION_DECRYPT;
	}

        if ((xEMI & 0xFF00) == 0x0000) {
		pCtrl->core = MT_CIPHER_CORE_M2M_TS;
        } else if ((xEMI & 0xFF00) == 0x4000) {
		pCtrl->core = MT_CIPHER_CORE_M2M_RAW;
        } else {
		return FALSE;
        }

	if (xEMI == NOCS_EMI_MPEG_TS_DVB_CSA2) {
		pCtrl->algorithm = MT_CIPHER_ALG_CSA2;
	} else if (xEMI == NOCS_EMI_MPEG_TS_DVB_CSA3) {
		pCtrl->algorithm = MT_CIPHER_ALG_CSA3;
	} else if ((xEMI & 0x00F0) == 0x0020) {
			pCtrl->algorithm = MT_CIPHER_ALG_AES;
	} else if ((xEMI & 0x00F0) == 0x0040) {
			pCtrl->algorithm = MT_CIPHER_ALG_TDES;
	} else {
		return FALSE;
	}

	return TRUE;
}

static TCsdStatus pcsdKeyLadder(
	MT_CIPHER_KEYLADDER_SOURCE_E root_key,
	const mt_u8 * pxL2CipheredProtectingKey,
	const mt_u8 * pxL1CipheredProtectingKey,
	const mt_u8 * pxCipheredContentKey,
	mt_u32 slot_id,
	MT_CIPHER_ALGORITHM_E cipher_algorithm)
{
    mt_s32 ret = -1;
    unsigned long kl_handle = 0;
    mt_u8 *p_kl_cmdsig = NULL;
#if 1 //hardkey1Òì»òOTP_KPHMACAuthKey, TEE-OS will set it, so after booting TEE-OS, shoulde use this group!
    mt_u8 kl_cmdsig[] = {
        0xBF,0x0D,0x48,0x0D,0xC0,0xA0,0x11,0xEC,
        0xDD,0x8E,0x6F,0xBC,0x94,0x4E,0x10,0xBB,
        0x3D,0xC5,0x8A,0x17,0x93,0x12,0x68,0xED,
        0xD0,0x8E,0x5C,0x07,0xE1,0xEC,0x8C,0x71,//SCK0 AESRootKey, 3, TDES
        0x17,0x45,0x53,0x0A,0x6F,0x72,0xED,0x52,
        0xAB,0xE5,0x92,0x99,0xD6,0x32,0x68,0x00,
        0x41,0xE9,0x9E,0x8E,0xA8,0x29,0x0D,0xC9,
        0x0A,0xC8,0x9A,0xCD,0x4B,0x8D,0xA8,0xA5,//SCK1 TDESRootKey, 3, TDES
        0xC7,0xA0,0xCB,0x82,0x5F,0x88,0xB6,0xD3,
        0xFF,0x89,0xD4,0x1B,0xEA,0x93,0x5F,0xC9,
        0x86,0x29,0xA5,0x66,0x12,0xC9,0x1E,0x34,
        0x55,0xBD,0xD2,0x99,0xFC,0x07,0xAE,0x37,//SCK2 CSA2RootKey, 3, TDES
        0xBF,0xD7,0x34,0x5B,0xFC,0x08,0xC2,0xC5,
        0x9A,0x26,0x75,0x96,0x2F,0x93,0x57,0xBD,
        0x61,0x4D,0x86,0xA1,0xD0,0x33,0x9C,0xF7,
        0x82,0x33,0x74,0xCF,0xAF,0xD1,0xD9,0x7A,//SCK3 CSA3RootKey, 3, TDES
        0x7D,0xD9,0x99,0x4E,0x14,0x4F,0xB6,0x06,
        0x16,0xDC,0x48,0xAD,0x6B,0x94,0xA8,0x99,
        0xD6,0x6E,0xC9,0x17,0xF0,0x8A,0x80,0xD6,
        0x8D,0xB5,0xA3,0x52,0x36,0xAD,0x1B,0xB6,//SCK4 CSK, 3, TDES
        0x71,0x5C,0xBD,0x98,0x15,0xB7,0xA7,0x5A,
        0xA6,0x1D,0x82,0x29,0x8B,0x60,0x32,0x6C,
        0xED,0xE7,0x26,0x6F,0x6A,0x39,0xB2,0xC2,
        0x09,0x92,0xDF,0x59,0x3D,0x84,0x99,0x9E,//SCK4 CSK, 3, AES
        0xF3,0xD8,0x26,0x0B,0x01,0x93,0xAD,0x47,
        0x66,0x42,0xEE,0x85,0x63,0xB9,0x85,0x66,
        0x76,0x0F,0xEA,0x46,0xF1,0xBB,0xCE,0x16,
        0x72,0xDB,0x9E,0xD9,0x14,0xF8,0x21,0xC3,//SCK5 LPPK, 3, TDES
        0x68,0x10,0xFC,0x4A,0xDD,0x7A,0x38,0x42,
        0xDA,0xCD,0x19,0xC1,0x35,0xB4,0x98,0x90,
        0xAC,0x75,0x44,0x70,0x7D,0x06,0x14,0x92,
        0x83,0x2A,0xCE,0xAB,0x08,0xFE,0xA4,0x8A,//SCK6 PVR, 3, AES
        0x3F,0x85,0x1D,0x89,0xC4,0x2E,0x1D,0x09,
        0x25,0xC4,0x15,0x8A,0xE5,0x53,0x77,0xD9,
        0xDD,0x03,0x81,0xFB,0xE1,0xF8,0xFF,0x01,
        0x81,0x5E,0xD1,0x31,0x24,0x48,0x05,0xCC,//SCK13 flashprotectRootKey, 3, TDES
        0x93,0xDC,0xF8,0xE9,0x84,0x44,0x2B,0xD7,
        0x4A,0x14,0x5D,0x41,0x2E,0x96,0x9C,0x1A,
        0x8B,0x3D,0x8C,0x53,0xCD,0x4B,0x37,0x6A,
        0xB6,0xBA,0xF8,0xCE,0xED,0xF6,0x96,0xDE,//SCK15 ETSI, 3, TDES
    };
#else //hw key0
    mt_u8 kl_cmdsig[] = {
        0xDB,0x95,0x54,0x24,0x55,0x5F,0x2D,0xC6,0x1E,0x45,0x6B,0x84,0x7B,0x05,0x4B,0x8C,
        0x56,0xFE,0x66,0x9B,0xD5,0xAD,0x82,0x2E,0x51,0x55,0xB3,0xE8,0xDA,0xFD,0xE2,0xAA,//SCK0 AESRootKey, 3, TDES
        0x0C,0xEC,0x61,0xCF,0xF6,0xE5,0x49,0x76,0x4A,0xBA,0xAC,0xF7,0x99,0xB8,0xD7,0x19,
        0x6D,0xDF,0xFB,0x06,0x09,0xBA,0xEB,0x3B,0x70,0x68,0x56,0xDE,0xC8,0x0C,0x0F,0x58,//SCK1 TDESRootKey, 3, TDES
        0xE0,0x4C,0x9C,0x1D,0x2C,0x96,0xA1,0x79,0xAC,0x49,0xA5,0x5D,0xE2,0x97,0x63,0xD0,
        0x76,0x96,0x6D,0xD0,0x01,0x2C,0xD7,0x4B,0xF5,0x9C,0x7D,0x23,0x69,0x08,0xD3,0x6C,//SCK2 CSA2RootKey, 3, TDES
        0x1B,0xA1,0x28,0x55,0xA6,0xBD,0x1D,0xC2,0xA1,0x35,0xAF,0x1D,0xFD,0x17,0x25,0xDF,
        0x74,0xD5,0x14,0x27,0x15,0x9B,0x41,0xD3,0x11,0xCA,0x7B,0x0C,0x1A,0x80,0xB9,0x19,//SCK3 CSA3RootKey, 3, TDES
        0x3E,0xE2,0x79,0xF1,0x49,0xD4,0xE7,0x47,0x23,0xB5,0xAB,0x33,0x68,0xBB,0x1D,0xE8,
        0x02,0x2F,0xAF,0x07,0x8D,0x70,0x3E,0x6B,0xA6,0x8F,0x40,0xBE,0xF9,0x66,0xC8,0xBF,//SCK4 CSK, 3, TDES
        0x79,0xF0,0x3F,0x93,0x2E,0xCE,0xF7,0xAE,0xF3,0x99,0x1D,0x4A,0xC6,0xA1,0x75,0x54,
        0xA4,0xAB,0x14,0x28,0x34,0xAB,0x86,0xA3,0xC6,0x3F,0xEC,0x47,0x31,0x2D,0x30,0x42,//SCK4 CSK, 3, AES
        0x46,0x62,0xE3,0xEF,0xC6,0xBE,0xA7,0x56,0xF9,0xE6,0x1D,0x75,0xF8,0xD8,0x5E,0xFB,
        0x34,0x7D,0x60,0xC2,0x69,0x56,0x82,0x95,0x30,0x1E,0x60,0xD6,0x97,0x06,0xBC,0xF2,//SCK5 LPPK, 3, TDES
        0xFF,0x4B,0x53,0x99,0xBF,0xEF,0xE1,0xD7,0x46,0x81,0x9F,0x57,0x4E,0xDF,0x42,0xAA,
        0xC5,0xC6,0xE2,0xB1,0x02,0x52,0xA1,0x33,0xB1,0x3B,0xC8,0x08,0xF1,0x3C,0x07,0xD7,//SCK6 PVR, 3, AES
        0x78,0x8F,0x95,0xD8,0x3C,0x78,0x5E,0x7D,0xE4,0xFA,0xB1,0x2C,0xB6,0xB1,0xD5,0xED,
        0x79,0xA6,0x82,0x2E,0x0A,0x0A,0xCC,0x50,0x4F,0xF0,0x01,0x64,0xF5,0xD5,0x3F,0x14,//SCK13 flashprotectRootKey, 3, TDES
        0x44,0x6D,0x9C,0x76,0xE9,0x3A,0x5A,0x59,0x55,0x5B,0x14,0x4C,0x93,0x47,0x80,0x82,
        0x5A,0xD8,0x8C,0xA8,0xE1,0xE1,0x1E,0xD6,0x67,0x3A,0xFC,0x95,0x19,0x55,0xB9,0xC0,//SCK15 ETSI, 3, TDES
    };
#endif
       //root_key = MT_CIPHER_KEYLADDER_SCK_13;

	if ((root_key > MT_CIPHER_KEYLADDER_SCK_6)
	    && (root_key != MT_CIPHER_KEYLADDER_SCK_13)
	    && (root_key != MT_CIPHER_KEYLADDER_SCK_15)) {
		CSD_ERR("KeyLadder unknown rootkey %d\n", root_key);
		return CSD_ERROR;
	}
	//temporary method cause of A0 not support AK=BK check for ContentKey
	if (!memcmp(pxCipheredContentKey, pxCipheredContentKey + 8, 8)) {
		CSD_ERR("KeyLadder ContentKey AK=BK found\n");
		return CSD_NO_ERROR;
		//return CSD_ERROR;
	}

       switch (root_key) {
	case MT_CIPHER_KEYLADDER_SCK_0:
	case MT_CIPHER_KEYLADDER_SCK_1:
	case MT_CIPHER_KEYLADDER_SCK_2:
	case MT_CIPHER_KEYLADDER_SCK_3:
		p_kl_cmdsig = kl_cmdsig + (32 * root_key);
		break;
	case MT_CIPHER_KEYLADDER_SCK_4:
		if (MT_CIPHER_ALG_TDES == cipher_algorithm) {
			p_kl_cmdsig = kl_cmdsig + 128;
		} else {
			p_kl_cmdsig = kl_cmdsig + 160;
		}
		break;
	case MT_CIPHER_KEYLADDER_SCK_5:
		p_kl_cmdsig = kl_cmdsig + 192;
		break;
	case MT_CIPHER_KEYLADDER_SCK_6:
		p_kl_cmdsig = kl_cmdsig + 224;
		break;
	case MT_CIPHER_KEYLADDER_SCK_13:	/* In nagra case, it is flash protection key */
		p_kl_cmdsig = kl_cmdsig + 256;
             break;
      case MT_CIPHER_KEYLADDER_SCK_15:
		p_kl_cmdsig = kl_cmdsig + 288;
		break;
	default:
		return CSD_ERROR;
		break;
	}
	ret = mt_unf_cipher_keyladder_create(MT_CIPHER_KEYLADDER_0, &kl_handle);
       CSD_DBG("kl_handle =  0x%lx \n", kl_handle);
	if (ret != 0) {
		CSD_ERR("\n");
		return CSD_ERROR;
	}

	ret = mt_unf_cipher_keyladder_start(kl_handle, root_key);
	if (ret != 0) {
		goto out;
	}
	CSD_DUMP("p_kl_cmdsig", p_kl_cmdsig, 32);
	ret |= mt_unf_cipher_keyladder_set_signature(kl_handle, p_kl_cmdsig);
	CSD_DBG("ret = 0x%x root_key = %d, cipher_algorithm = 0x%x, slot_id = %d, kl_handle = %lx \n",
            ret, root_key, cipher_algorithm, slot_id, kl_handle);

	MT_CIPHER_CTRL_S s_ctrl = { 0 };
	s_ctrl.core = MT_CIPHER_CORE_M2M_RAW;
	s_ctrl.algorithm = cipher_algorithm;
	s_ctrl.operation = MT_CIPHER_OPERATION_DECRYPT;
	s_ctrl.work_mode = MT_CIPHER_WORK_MODE_ECB;
	ret |= mt_unf_cipher_keyladder_link(kl_handle, &s_ctrl,
				(unsigned char *)pxL2CipheredProtectingKey, 16);
	ret |= mt_unf_cipher_keyladder_link(kl_handle, &s_ctrl,
				(unsigned char *)pxL1CipheredProtectingKey, 16);
	ret |= mt_unf_cipher_keyladder_link(kl_handle, &s_ctrl,
				(unsigned char *)pxCipheredContentKey, 16);
	if (ret != 0) {
		goto out;
	}


	ret |= mt_unf_cipher_keyladder_end(kl_handle, slot_id);
	if (ret != 0) {
		CSD_ERR("ret = 0x%x \n", ret);
		goto out;
	}

out:
	mt_unf_cipher_keyladder_destroy(kl_handle);
	if (ret != 0) {
		CSD_ERR("KeyLadder SCK%d to slot %d\n", root_key, slot_id);
		/* return CSD_ERROR; */
		return CSD_NO_ERROR;
	}
	CSD_INFO("KeyLadder SCK%d to slot %d success\n", root_key, slot_id);
	CSD_DUMP("pxL2CipheredProtectingKey", (mt_u8 *)pxL2CipheredProtectingKey, 16);
	CSD_DUMP("pxL1CipheredProtectingKey", (mt_u8 *)pxL1CipheredProtectingKey, 16);
	CSD_DUMP("pxCipheredContentKey", (mt_u8 *)pxCipheredContentKey, 16);
	return CSD_NO_ERROR;
}


static TCsdStatus mt_flash_read_test(
	unsigned int mtddev,
	TCsdSize xAddress,
	TCsdSize xDataSize,
	const TCsdUnsignedInt8 * pxData)
{
	mt_handle gmtd_handle = 0;
	TCsdStatus ret = CSD_ERROR;
	char mtddev_name[64] = { 0 };

	sprintf(mtddev_name, "/dev/mtd%d", mtddev);
	CSD_INFO("csd MTCommand_Flash_Init mtddev_name:%s\n", mtddev_name);
	mt_unf_flash_init();

	ret = mt_unf_flash_open(mtddev_name, &gmtd_handle);
	if (0 != ret) {
		CSD_INFO("csd MTCommand_Flash_Init mt_unf_flash_open error ret = %d\n",
			ret);
		mt_unf_flash_deinit();
		return CSD_ERROR;
	}

	ret = mt_unf_flash_read(gmtd_handle, xAddress, (unsigned char *)pxData, xDataSize);
	if (0 != ret) {
		CSD_INFO("csd MTCommand_Flash_Init mt_unf_flash_read error ret = %d, 0x%lx, 0x%lx \n",
		    ret, xAddress, xDataSize);
		mt_unf_flash_deinit();
		return CSD_ERROR;
	}

	mt_unf_flash_close(gmtd_handle);
	return CSD_NO_ERROR;

}

static TCsdStatus mt_flash_read_bulk(
	TCsdSize xAddress,
	TCsdSize xDataSize,
	const TCsdUnsignedInt8 * pxData)
{
	//unsigned int mtddev = DEFAULT_MTDDEV, i = 0;
	TCsdStatus ret = CSD_ERROR;

	//TCsdSize xDataSize_temp = 0;
	CSD_INFO("xAddress = 0x%lx, xDataSize = 0x%lx \n", xAddress, xDataSize);
	mt_unf_flash_init();
	//just for test harness test, now!driver should supply a api to get mtdev by address!!!
	//mtdparts=mt_snf:128k(btinit.img),512k(boot.img),128k(boot.scr),256k(av_cpu.img),1152k(logo.img),9M(kernel.img),20M(usrfs.img),20M(data.img)
	//mtdparts=mt_sf:1536k(btinit_uboot),64k(boot.scr),640k(av_cpu.img),4608k(kernel.img),5120k(rootfs),3904k(usrfs.img),384k(datafs.jffs2),128k(logo)
	if (xAddress < 0x20000)	//read parameter
	{
		//read parameter
		ret = mt_flash_read_test(0, xAddress, xDataSize, pxData);
		//hex_dump("flash data:", pxData, xDataSize);
		if (ret) {
			CSD_INFO("csd mt_flash_read_bulk error ret = %d\n",
				 ret);
			return CSD_ERROR;
		}
	} else {
		CSD_INFO("csd flash read error 1111 !\n");
		return CSD_ERROR;
	}
	CSD_INFO("csd flash read success !\n");
	return CSD_NO_ERROR;
}

static TCsdStatus csd_pv_get_otp_mask(
	TCsdPvId pv_id,
	TCsdUnsignedInt32 mask_size,
	TCsdUnsignedInt8 * mask)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(pv_otp_map); i++) {
		if (pv_otp_map[i].pv_id == pv_id)
			break;
	}

	RETURN_VAL_IF_FAIL(i < ARRAY_SIZE(pv_otp_map),
		CSD_ERROR_OPERATION_NOT_SUPPORTED);
	RETURN_VAL_IF_FAIL(pv_otp_map[i].otp_size <= mask_size,
		CSD_ERROR_INVALID_PARAMETERS);

	memset(mask, 0, mask_size);
	memcpy(mask, &pv_otp_map[i].otp_mask, pv_otp_map[i].otp_size);
	return CSD_NO_ERROR;
}

static TCsdStatus csd_pv_get_otp_value(
	TCsdPvId pv_id,
	TCsdUnsignedInt32 value_size,
	TCsdUnsignedInt8 * value)
{
	size_t i;
	TCsdStatus status = CSD_ERROR;

	for (i = 0; i < ARRAY_SIZE(pv_otp_map); i++) {
		if (pv_otp_map[i].pv_id == pv_id)
			break;
	}

	RETURN_VAL_IF_FAIL(i < ARRAY_SIZE(pv_otp_map),
		CSD_ERROR_OPERATION_NOT_SUPPORTED);
	RETURN_VAL_IF_FAIL(pv_otp_map[i].otp_size <= value_size,
		CSD_ERROR_INVALID_PARAMETERS);

	if (mt_otp_read_byte(pv_otp_map[i].otp_offset, pv_otp_map[i].otp_size,
		(MT_U8 *) value) != MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}

	if (pv_otp_map[i].reverse) {
		pcsdByteSwap(value, value_size);
	}

	status = CSD_NO_ERROR;
      OUT:
	return status;
}

static TCsdStatus csd_pv_set_otp_value(
	TCsdPvId pv_id,
	TCsdUnsignedInt32 value_size,
	const TCsdUnsignedInt8 * value)
{
	size_t i;
	TCsdStatus status = CSD_ERROR;

	for (i = 0; i < ARRAY_SIZE(pv_otp_map); i++) {
		if (pv_otp_map[i].pv_id == pv_id)
			break;
	}

	RETURN_VAL_IF_FAIL(i < ARRAY_SIZE(pv_otp_map),
		CSD_ERROR_OPERATION_NOT_SUPPORTED);
	RETURN_VAL_IF_FAIL(pv_otp_map[i].otp_size <= value_size,
		CSD_ERROR_INVALID_PARAMETERS);

	TCsdUnsignedInt8 buf[value_size];

	memcpy(buf, value, value_size);
	if (pv_otp_map[i].reverse) {
		pcsdByteSwap(buf, value_size);
	}

	if (mt_otp_write_byte(pv_otp_map[i].otp_offset, pv_otp_map[i].otp_size,
		(MT_U8 *) buf) != MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}

	status = CSD_NO_ERROR;
      OUT:
	return status;
}

static TCsdStatus csd_pv_get_const_value(
	TCsdPvId pv_id,
	TCsdUnsignedInt32 value_size,
	TCsdUnsignedInt8 * value)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(pv_const_map); i++) {
		if (pv_const_map[i].pv_id == pv_id)
			break;
	}

	RETURN_VAL_IF_FAIL(i < ARRAY_SIZE(pv_const_map),
		CSD_ERROR_OPERATION_NOT_SUPPORTED);
	RETURN_VAL_IF_FAIL(pv_const_map[i].pv_size <= value_size,
		CSD_ERROR_INVALID_PARAMETERS);

	memcpy(value, &pv_const_map[i].pv_value, pv_const_map[i].pv_size);
	return CSD_NO_ERROR;
}


static TCsdStatus CsdSetPv(
	TCsdPvId xPvId,
	TCsdUnsignedInt32 xPvBufferSize,
	const TCsdUnsignedInt8 * pxPvBufferValue)
{
	TCsdStatus status = CSD_ERROR;

	RETURN_VAL_IF_FAIL(xPvBufferSize > 0, CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxPvBufferValue != NULL,
			   CSD_ERROR_INVALID_PARAMETERS);

	CSD_INFO("xPvId = %ld, xPvBufferSize = %ld\n", xPvId, xPvBufferSize);

	status = csd_pv_set_otp_value(xPvId, xPvBufferSize, pxPvBufferValue);

	CSD_DUMP("pxPvBufferValue", (mt_u8 *)pxPvBufferValue, xPvBufferSize);
	CSD_INFO("status = %d\n", status);
	return status;
}

static TCsdStatus CsdGetPv(
	TCsdPvId xPvId,
	TCsdUnsignedInt32 xPvBufferSize,
	TCsdUnsignedInt8 * pxPvBufferValue)
{
	TCsdStatus status = CSD_ERROR;

	RETURN_VAL_IF_FAIL(xPvBufferSize > 0, CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxPvBufferValue != NULL,
			   CSD_ERROR_INVALID_PARAMETERS);

	CSD_INFO("xPvId = %ld, xPvBufferSize = %ld\n", xPvId, xPvBufferSize);

	status = csd_pv_get_const_value(xPvId, xPvBufferSize, pxPvBufferValue);
	if (status != CSD_NO_ERROR) {
		status = csd_pv_get_otp_value(xPvId, xPvBufferSize, pxPvBufferValue);
		if (status == CSD_NO_ERROR) {
			TCsdUnsignedInt8 mask[xPvBufferSize];
			TCsdUnsignedInt32 i;

			csd_pv_get_otp_mask(xPvId, xPvBufferSize, mask);
			for (i = 0; i < xPvBufferSize; i++) {
				pxPvBufferValue[i] &= mask[i];
			}
		} else {
			//not used anymore, only get pv from OTP
			//status = csd_pv_get_flash_value(xPvId, xPvBufferSize, pxPvBufferValue);
		}
	}

	CSD_DUMP("pxPvBufferValue", (mt_u8 *)pxPvBufferValue, xPvBufferSize);
	CSD_INFO("status = %d\n", status);
	return status;
}

TCsdStatus csdInitialize(TCsdInitParameters * pxInitParameters)
{
	csd_set_log_level(CSD_LOG_INFO);
	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited != MT_TRUE, CSD_ERROR_DRIVER_ALREADY_INITIALIZED);
	RETURN_VAL_IF_FAIL(pxInitParameters != NULL, CSD_ERROR_INVALID_PARAMETERS);
	mt_sys_init();
	mt_unf_cipher_init();
	CSD_INFO("\n");
	csd_inited = MT_TRUE;

	return CSD_NO_ERROR;
}

TCsdStatus csdTerminate(TCsdTerminateParameters * pxTerminateParameters)
{
	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_NO_ERROR);
	RETURN_VAL_IF_FAIL(pxTerminateParameters != NULL, CSD_ERROR_INVALID_PARAMETERS);

	mt_unf_cipher_deinit();
	mt_sys_deinit();
	csd_inited = MT_FALSE;

	return CSD_NO_ERROR;
}

TCsdStatus csdGetApiVersion(TCsdUnsignedInt32 * pxCsdApiVersion)
{
	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(pxCsdApiVersion != NULL, CSD_ERROR_INVALID_PARAMETERS);

	*pxCsdApiVersion = CSDAPI_VERSION_INT;
	CSD_INFO("*pxCsdApiVersion = 0x%lx\n", *pxCsdApiVersion);
	return CSD_NO_ERROR;
}

TCsdStatus csdGetSoftwareVersion(TCsd20CharsString xSoftwareVersion)
{
	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xSoftwareVersion != NULL, CSD_ERROR_INVALID_PARAMETERS);

	snprintf((char *)xSoftwareVersion, sizeof(TCsd20CharsString), "CSDAPI_%d.%d.%d",
		CSDAPI_VERSION_MAJOR, CSDAPI_VERSION_MEDIUM,
		CSDAPI_VERSION_MINOR);

	CSD_INFO("xSoftwareVersion = %s\n", xSoftwareVersion);
	return CSD_NO_ERROR;
}

#define CSD_R_CHIP_ID_RDEN		(0xBF140008)
#define CSD_R_CHIP_ID		(0xBF140004)
/*
#define OTP_FUSE_VER_OFFSET		(0xE8C << 2)
#define OTP_FUSE_VER_SIZE                              (4)
#define OTP_FUSE_VER_SHIFT                             (0)
#define OTP_FUSE_VER_BITS                              (32)
*/
TCsdStatus csdGetChipRevision(TCsd20CharsString xChipRevision)
{
	CHIP_REVISION_PAIR chip_rev;
	unsigned int data = 0;

	CSD_INFO("\n");
#if 0
        memset((char *)chip_rev.chars, 0x00, 4);
        strcpy((char *)chip_rev.chars, (const char *)"A0");
        memcpy(xChipRevision, chip_rev.chars, 4);
        CSD_INFO("xChipRevision = %s CSD_NO_ERROR=0x%x\n", xChipRevision,
		CSD_NO_ERROR);
#else
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xChipRevision != NULL, CSD_ERROR_INVALID_PARAMETERS);

	//mt_sys_write_register must use phisical address
	mt_sys_write_register(CSD_R_CHIP_ID_RDEN, 0xffff);
	//must use phisical address
	mt_sys_read_register(CSD_R_CHIP_ID, (mt_u32 *)&data);
	CSD_INFO("data = 0x%lx\n", data);

	chip_rev.data = data & 0xffff;
 /*
	if (mt_otp_read_byte(OTP_FUSE_VER_OFFSET, OTP_FUSE_VER_SIZE,
			(MT_U8 *) & otp_fusever) != MT_SUCCESS) {
			return CSD_ERROR;
	}
	CSD_INFO("otp_fusever = 0x%lx\n", otp_fusever);
*/
	memset(xChipRevision, 0x00, sizeof(TCsd20CharsString));
	chip_rev.chars[2] = 0;
	chip_rev.chars[3] = 0;
#if 1
	switch (data) {
	case 0xB000:
		strcpy((char *)chip_rev.chars, (const char *)"A0");
		break;
	case 0xB001:
		strcpy((char *)chip_rev.chars, (const char *)"A1");
		break;
	default:
		return CSD_ERROR;
	}
#else
    strcpy((char *)chip_rev.chars, (const char *)"A1"); //all test is A0
#endif
	memcpy(xChipRevision, chip_rev.chars, 4);
	CSD_INFO("xChipRevision = %s CSD_NO_ERROR=0x%x\n", xChipRevision,
		CSD_NO_ERROR);
#endif
	return CSD_NO_ERROR;
}

#define OTP_CHIP_INFOR_1			(0xBF313FC4)
#define OTP_PACKAGEINFO_OFFSET	(28)
#define OTP_SPIDRAM_OFFSET		(24)
#define OTP_CAVENDOR_OFFSET		(18)
#define OTP_CAVERSION_OFFSET		(14)
#define OTP_DRMINFO_OFFSET		(10)
#define OTP_IPLICENSE_OFFSET		(6)

TCsdStatus csdGetChipExtension(TCsd20CharsString xChipExtension)
{
#if 0
	const char *test_p = "BxG0xx";
	memset(xChipExtension, 0x00, sizeof(TCsd20CharsString));
	memcpy(xChipExtension, test_p, 6);	//for test
	CSD_INFO("xChipExtension = %s \n", xChipExtension);
#else
	unsigned int chip_infor;
	unsigned char p_ch[20] = {0,};
	unsigned char temp0;
	static unsigned char package_infor[16] = { 'Q', 'R', 'S',0x00, 'N', 'O', 'P', 0x00, 'B', 'C', 'D'};
	//unsigned char drm_size[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', 0x00, 0x00, 'B', 'C', 'D', 'E' };
	unsigned char ca_vendor[32] = {0x00, 'G', 'C', 'T', 'A', 'U', 'P', 'V', 'D', 'Y', 'Z', 'S'};

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xChipExtension != NULL, CSD_ERROR_INVALID_PARAMETERS);

	memset(p_ch, 0x0, sizeof(p_ch));
	memset(xChipExtension, 0x0, sizeof(TCsd20CharsString));
	if (mt_otp_read_byte
		((OTP_CHIP_INFOR_1 & 0xFFFF), OTP_NUIDL_SIZE,
		(MT_U8 *) & chip_infor) != MT_SUCCESS) {
		return CSD_ERROR;
	}
	CSD_INFO("chip_infor = 0x%x \n", chip_infor);
	temp0 = (chip_infor >> OTP_PACKAGEINFO_OFFSET) & 0xF;
	p_ch[0] = package_infor[temp0];
	temp0 = (chip_infor >> OTP_SPIDRAM_OFFSET) & 0xF;
	//p_ch[1] = drm_size[temp0];
	p_ch[1] = 'x';/* update according to Nagra e-mail request */
	temp0 = (chip_infor >> OTP_CAVENDOR_OFFSET) & 0x3F;
	p_ch[2] = ca_vendor[temp0];
    	if (p_ch[2] != 'G') {
		//it must be Nagra project!
		return CSD_ERROR;
	}
	temp0 = (chip_infor >> OTP_CAVERSION_OFFSET) & 0xF;
	p_ch[3] = '0' + temp0;
	p_ch[4] = 'x';/* update according to Nagra e-mail request */
	p_ch[5] = 'x';/* update according to Nagra e-mail request */

	strcpy((char *)xChipExtension, (const char *)p_ch);

	CSD_INFO("%s \n", xChipExtension);
 #endif
	return CSD_NO_ERROR;
}

TCsdStatus csdGetNuid(TCsd4BytesVector xNuid)
{
	TCsdStatus status = CSD_ERROR;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xNuid != NULL, CSD_ERROR_INVALID_PARAMETERS);

	if (mt_otp_read_byte(OTP_NUIDL_OFFSET, OTP_NUIDL_SIZE, (MT_U8 *) xNuid)
		!= MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}
	pcsdByteSwap(xNuid, 4);

	status = CSD_NO_ERROR;
OUT:
	CSD_DUMP((char *)"xNuid", (MT_U8 *) xNuid, sizeof(TCsd4BytesVector));
	CSD_INFO("status = %d\n", status);
	return status;
}

TCsdStatus csdGetNuid64(TCsd8BytesVector xNuid64)
{
	TCsdStatus status = CSD_ERROR;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xNuid64 != NULL, CSD_ERROR_INVALID_PARAMETERS);

	if ((mt_otp_read_byte(OTP_NUIDL_OFFSET, OTP_NUIDL_SIZE,
		(MT_U8 *) xNuid64) != MT_SUCCESS) ||
		(mt_otp_read_byte(OTP_NUIDH_OFFSET, OTP_NUIDH_SIZE,
		(MT_U8 *) (xNuid64 + 4)) != MT_SUCCESS)) {
		status = CSD_ERROR;
		goto OUT;
	}

	status = CSD_NO_ERROR;
	pcsdByteSwap(xNuid64, 8);
OUT:
	CSD_DUMP("xNuid", xNuid64, sizeof(TCsd4BytesVector));
	CSD_INFO("status = %d\n", status);
	return status;
}

#define NUID_CHECK_NUM		(0xbf314200)

TCsdStatus csdGetNUIDCheckNumber(TCsd4BytesVector xNUIDCheckNumber)
{
	TCsdStatus status = CSD_ERROR;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xNUIDCheckNumber != NULL, CSD_ERROR_INVALID_PARAMETERS);

	if (mt_sys_read_register(NUID_CHECK_NUM, (MT_U32 *) xNUIDCheckNumber) !=
		MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}
	status = CSD_NO_ERROR;
      OUT:

	pcsdByteSwap(xNUIDCheckNumber, 4);

	CSD_DUMP("xNuid", xNUIDCheckNumber, sizeof(TCsd4BytesVector));
	CSD_INFO("status = %d\n", status);
	return status;
}

TCsdStatus csdGetCSCDCheckNumber(
	const TCsdUnsignedInt8 xCSCData[16],
	TCsd4BytesVector xCSCDCheckNumber)
{
	TCsdUnsignedInt8 buf[16];
	mt_u32 crc;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xCSCData != NULL, CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(xCSCDCheckNumber != NULL, CSD_ERROR_INVALID_PARAMETERS);

	CSD_DUMP("xCSCData", (mt_u8 *)xCSCData, 16);

	if (csdEncryptDataWithSecretContentKey(CSD_R2R_ALGORITHM_TDES_K1K2K1,
		CSD_R2R_CRYPTO_OPERATION_MODE_ECB,
		kl_prot_keys,
		(const TCsdUnsignedInt8 *)
		kl_prot_keys, 16, NULL, 0, TRUE,
		NULL, xCSCData, buf,
		16) != CSD_NO_ERROR)
		return CSD_ERROR;

	crc = crc32_calc(buf, sizeof(buf), 0);
	memcpy(xCSCDCheckNumber, &crc, 4);
	pcsdByteSwap(xCSCDCheckNumber, 4);

	CSD_DUMP("xCSCDCheckNumber", xCSCDCheckNumber, 4);
	return CSD_NO_ERROR;
}

TCsdStatus csdGetSTBCASNCheckNumber(
	const TCsdUnsignedInt8 xSTBCASNData[4],
	TCsd4BytesVector xSTBCASNCheckNumber)
{
	TCsdUnsignedInt8 buf[16] = { 0 };
	mt_u32 crc;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xSTBCASNData != NULL, CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(xSTBCASNCheckNumber != NULL,	CSD_ERROR_INVALID_PARAMETERS);

	CSD_DUMP("xSTBCASNData", (mt_u8 *)xSTBCASNData, 4);

	memcpy(buf + 12, xSTBCASNData, 4);
	if (csdEncryptDataWithSecretContentKey(CSD_R2R_ALGORITHM_TDES_K1K2K1,
		CSD_R2R_CRYPTO_OPERATION_MODE_ECB,
		kl_prot_keys,
		(const TCsdUnsignedInt8 *)
		kl_prot_keys, 16, NULL, 0, TRUE,
		NULL, buf, buf,
		16) != CSD_NO_ERROR)
		return CSD_ERROR;

	crc = crc32_calc(buf, sizeof(buf), 0);
	memcpy(xSTBCASNCheckNumber, &crc, 4);
	pcsdByteSwap(xSTBCASNCheckNumber, 4);

	CSD_DUMP("xSTBCASNCheckNumber", xSTBCASNCheckNumber, 4);
	return CSD_NO_ERROR;
}

TCsdStatus csdGetDataIntegrityCheckNumber(
	const TCsdUnsignedInt8 * pxData,
	TCsdSize xDataSize,
	TCsd4BytesVector
	xDataIntegrityCheckNumber)
{
	TCsdUnsignedInt8 buf[16] = { 0 };
	mt_u32 crc;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(pxData != NULL, CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL((xDataSize > 0)&& (xDataSize <= 16),
		CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(xDataIntegrityCheckNumber != NULL,
		CSD_ERROR_INVALID_PARAMETERS);

	CSD_DUMP("pxData", (mt_u8 *)pxData, xDataSize);

	memcpy(buf + 16 - xDataSize, pxData, xDataSize);
	if (csdEncryptDataWithSecretContentKey(CSD_R2R_ALGORITHM_TDES_K1K2K1,
		CSD_R2R_CRYPTO_OPERATION_MODE_ECB,
		kl_prot_keys,
		(const TCsdUnsignedInt8 *)
		kl_prot_keys, 16, NULL, 0, TRUE,
		NULL, buf, buf,
		16) != CSD_NO_ERROR)
		return CSD_ERROR;

	crc = crc32_calc(buf, sizeof(buf), 0);
	memcpy(xDataIntegrityCheckNumber, &crc, 4);
	pcsdByteSwap(xDataIntegrityCheckNumber, 4);
	CSD_DUMP("xDataIntegrityCheckNumber", xDataIntegrityCheckNumber, 4);
	return CSD_NO_ERROR;
}

TCsdStatus csdSetStbCaSn(const TCsd4BytesVector xStbCaSn)
{
	MT_U32 data;
	MT_U8 lock_data;
	TCsdStatus status = CSD_ERROR;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xStbCaSn != NULL, CSD_ERROR_INVALID_PARAMETERS);

	CSD_DUMP("xStbCaSn 000:\n", (mt_u8 *)xStbCaSn, 4);
	pcsdByteSwap((TUnsignedInt8 *) xStbCaSn, 4);
	if (mt_otp_read_byte(OTP_SWL_STB_CA_SN_OFFSET, OTP_SWL_STB_CA_SN_SIZE,
		(MT_U8 *) & data) != MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}
	if ((data & (0x7 << OTP_SWL_STB_CA_SN_SHIFT)) !=
		OTP_UNLOCK_STB_CA_SN_VALUE) {
		if (mt_otp_read_byte(OTP_STB_CA_SN_OFFSET, OTP_STB_CA_SN_SIZE,
			(MT_U8 *) & data) != MT_SUCCESS) {
			status = CSD_ERROR;
			goto OUT;
		}
		if (memcmp(xStbCaSn, (MT_U8 *) & data, 4)) {
			status = CSD_ERROR_OPERATION_NOT_ALLOWED;
			goto OUT;
		} else {
			status = CSD_NO_ERROR;
			goto OUT;
		}
	}
	if (mt_otp_write_byte(OTP_STB_CA_SN_OFFSET, OTP_STB_CA_SN_SIZE,
		(MT_U8 *) xStbCaSn) != MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}
	lock_data = OTP_LOCK_STB_CA_SN_VALUE;
	if (mt_otp_write_byte(OTP_SWL_STB_CA_SN_OFFSET, OTP_SWL_STB_CA_SN_SIZE,
		&lock_data) != MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}
	status = CSD_NO_ERROR;

OUT:
	CSD_DUMP("xStbCaSn", (mt_u8 *)xStbCaSn, sizeof(TCsd4BytesVector));
	CSD_INFO("status = %d\n", status);
	return status;
}

TCsdStatus csdGetStbCaSn(TCsd4BytesVector xStbCaSn)
{
	TCsdStatus status = CSD_ERROR;
	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xStbCaSn != NULL, CSD_ERROR_INVALID_PARAMETERS);

	if (mt_otp_read_byte(OTP_STB_CA_SN_OFFSET, OTP_STB_CA_SN_SIZE,
		(MT_U8 *) xStbCaSn) != MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}

	status = CSD_NO_ERROR;
OUT:
	pcsdByteSwap(xStbCaSn, 4);
	CSD_DUMP("xStbCaSn", xStbCaSn, sizeof(TCsd4BytesVector));
	CSD_INFO("status = %d\n", status);
	return status;
}

TCsdStatus csdSetMarketSegmentId(const TCsd4BytesVector xMarketSegmentId)
{
	MT_U32 data;
	MT_U8 lock_data;
	TCsdStatus status = CSD_ERROR;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xMarketSegmentId != NULL,
			   CSD_ERROR_INVALID_PARAMETERS);

	CSD_DUMP("xMarketSegmentId000:\n", (mt_u8 *)xMarketSegmentId, 4);

	pcsdByteSwap((TUnsignedInt8 *) xMarketSegmentId, 4);
	if (mt_otp_read_byte(OTP_SWL_MSID0_OFFSET, OTP_SWL_MSID0_SIZE,
		(MT_U8 *) & data) != MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}
	if ((data & (0x7 << OTP_SWL_MSID0_SHIFT)) != OTP_UNLOCK_MSID0_VALUE) {
		if (mt_otp_read_byte(OTP_MSID0_OFFSET, OTP_MSID0_SIZE,
			(MT_U8 *) & data) != MT_SUCCESS) {
			status = CSD_ERROR;
			goto OUT;
		}
		if (memcmp(xMarketSegmentId, (MT_U8 *) & data, 4)) {
			status = CSD_ERROR_OPERATION_NOT_ALLOWED;
			goto OUT;
		} else {
			status = CSD_NO_ERROR;
			goto OUT;
		}
	}
	if (mt_otp_write_byte(OTP_MSID0_OFFSET, OTP_MSID0_SIZE,
		(MT_U8 *) xMarketSegmentId) != MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}
	lock_data = OTP_LOCK_MSID0_VALUE;
	if (mt_otp_write_byte(OTP_SWL_MSID0_OFFSET, OTP_SWL_MSID0_SIZE,
		&lock_data) != MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}
	status = CSD_NO_ERROR;

OUT:
	CSD_DUMP("xMarketSegmentId", (mt_u8 *)xMarketSegmentId,
		sizeof(TCsd4BytesVector));
	CSD_INFO("status = %d\n", status);
	return status;
}

TCsdStatus csdGetMarketSegmentId(TCsd4BytesVector xMarketSegmentId)
{
	TCsdStatus status = CSD_ERROR;
	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xMarketSegmentId != NULL,
		CSD_ERROR_INVALID_PARAMETERS);

	if (mt_otp_read_byte(OTP_MSID0_OFFSET, OTP_MSID0_SIZE,
		(MT_U8 *) xMarketSegmentId) != MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}
	status = CSD_NO_ERROR;
      OUT:
	pcsdByteSwap(xMarketSegmentId, 4);
	CSD_DUMP("xMarketSegmentId", xMarketSegmentId,
		sizeof(TCsd4BytesVector));
	CSD_INFO("status = %d\n", status);
	return status;
}

TCsdStatus csdSetBootMode(TCsdBootMode xBootMode)
{

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);

	CSD_INFO("xBootMode = %d\n", xBootMode);
	return CSD_ERROR_OPERATION_NOT_SUPPORTED;
}

TCsdStatus csdGetBootMode(TCsdBootMode * pxBootMode)
{
	TCsdUnsignedInt32 data = 0;
	MT_U32 cfg = 0;
	TCsdStatus status = CSD_ERROR;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(pxBootMode != NULL, CSD_ERROR_INVALID_PARAMETERS);

	if (mt_otp_read_byte(OTP_BOOT_MODE_SEL_OFFSET, OTP_BOOT_MODE_SEL_SIZE,
		(MT_U8 *) & data) != MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}
	CSD_INFO("data=0x%lx\n", data);
	if (BOOT_MODE_SEL_IS_STRAP_PIN(data)) {
		if (mt_sys_read_register(REG_BOOT_CFG, &cfg) != MT_SUCCESS) {
			status = CSD_ERROR;
			goto OUT;
		}
		//*pxBootMode = ((cfg & 0x4) == 0) ? CSD_BOOT_SPI_NOR: CSD_BOOT_SPI_NAND;
		CSD_INFO("cfg=0x%x\n", cfg);
		if ((cfg & 0x4) == 0)
			*pxBootMode = CSD_BOOT_SPI_NOR;
		else if ((cfg & 0x7) == 0x7)
			*pxBootMode = CSD_BOOT_PNAND;
		else if ((cfg & 0x7) == 0x4)
			*pxBootMode = CSD_BOOT_SPI_NAND;

		CSD_INFO("*pxBootMode=0x%x\n", *pxBootMode);

	} else if (BOOT_MODE_SEL_IS_SPI_NOR(data)) {
		CSD_INFO("cfg=0x%x\n", cfg);
		*pxBootMode = CSD_BOOT_SPI_NOR;
	} else if (BOOT_MODE_SEL_IS_PNAND(data)) {
		*pxBootMode = CSD_BOOT_PNAND;
		CSD_INFO("cfg=0x%x\n", cfg);
	} else if (BOOT_MODE_SEL_IS_SPI_NAND(data)) {
		*pxBootMode = CSD_BOOT_SPI_NAND;
		CSD_INFO("cfg=0x%x\n", cfg);
	} else if (BOOT_MODE_SEL_IS_EMMC(data)) {
		*pxBootMode = CSD_BOOT_SPI_EMMC;
		CSD_INFO("cfg=0x%x\n", cfg);
	} else {
		status = CSD_ERROR;
		goto OUT;
	}
	status = CSD_NO_ERROR;
OUT:
	CSD_INFO("*pxBootMode = %d, status = %d\n", *pxBootMode, status);
	return status;
}

TCsdStatus csdEnableScs(void)
{
	TCsdUnsignedInt8 value = 0xff;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);

	if (CsdSetPv(CSD_NOCS_PVID_SCS_ACTIVATION_ID, sizeof(value), &value) !=
	    CSD_NO_ERROR)
		return CSD_ERROR;

	return CSD_NO_ERROR;
}

TCsdStatus csdGetScsActivationFlag(
	TCsdActivationFlag* pxScsActivationFlagState)
{
	TCsdUnsignedInt8 value = 0;
	TCsdStatus status = CSD_ERROR;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(pxScsActivationFlagState != NULL, CSD_ERROR_INVALID_PARAMETERS);

	if ((status = CsdGetPv(CSD_NOCS_PVID_SCS_ACTIVATION_ID, sizeof(value),
		&value)) != CSD_NO_ERROR)
		return status;

	if (value > 0)
		*pxScsActivationFlagState = CSD_ACTIVATION_FLAG_SET;
	else
		*pxScsActivationFlagState = CSD_ACTIVATION_FLAG_NOT_SET;
	CSD_INFO("*pxScsActivationFlagState = %d\n", *pxScsActivationFlagState);
	return CSD_NO_ERROR;
}

TCsdStatus csdSetScsPv(
	TCsdScsPvId xScsPvParamsId,
	TCsdScsPvPathHandle * pxPvPathHandle,
	TCsdUnsignedInt16 xPvLength,
	TCsdUnsignedInt8 * pxPvValue)
{
	size_t i;
	TCsdStatus status;
	TCsdUnsignedInt32 size;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xPvLength > 0, CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxPvValue != NULL, CSD_ERROR_INVALID_PARAMETERS);

	CSD_INFO("xScsPvParamsId = %d, xPvLength = %d\n", 	xScsPvParamsId, xPvLength);
	CSD_DUMP("pxPvValue", pxPvValue, xPvLength);

	for (i = 0; i < ARRAY_SIZE(csd_pv_map); i++) {
		if (csd_pv_map[i].csd_id == xScsPvParamsId)
			break;
	}

	RETURN_VAL_IF_FAIL(i < ARRAY_SIZE(csd_pv_map),
		CSD_ERROR_OPERATION_NOT_SUPPORTED);

	TCsdUnsignedInt8 buf[xPvLength];

	size = xPvLength;
	memcpy(buf, pxPvValue, xPvLength);
	if ((status = CsdSetPv(csd_pv_map[i].ctd_id, size, buf)) != CSD_NO_ERROR) {
		return status;
	}

	if (CsdGetPv(csd_pv_map[i].ctd_id, size, buf) != CSD_NO_ERROR) {
		return CSD_ERROR;
	}

	if (memcmp(buf, pxPvValue, xPvLength)) {
		return CSD_ERROR_OPERATION_NOT_ALLOWED;
	}

	CSD_INFO("\n");
	return CSD_NO_ERROR;
}

TCsdStatus csdGetScsPv(TCsdScsPvId xScsPvParamsId,
		       TCsdScsPvPathHandle * pxPvPathHandle,
		       TCsdUnsignedInt16 xPvLength,
		       TCsdUnsignedInt8 * pxPvValue)
{
	size_t i;
	TCsdStatus status;
	TCsdUnsignedInt32 size;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xPvLength > 0, CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxPvValue != NULL, CSD_ERROR_INVALID_PARAMETERS);

	CSD_INFO("xScsPvParamsId = %d, xPvLength = %d\n", xScsPvParamsId, xPvLength);
	for (i = 0; i < ARRAY_SIZE(csd_pv_map); i++) {
		if (csd_pv_map[i].csd_id == xScsPvParamsId)
			break;
	}

	RETURN_VAL_IF_FAIL(i < ARRAY_SIZE(csd_pv_map),
		CSD_ERROR_OPERATION_NOT_SUPPORTED);

	TCsdUnsignedInt8 buf[xPvLength];

	size = xPvLength;
	if ((status = CsdGetPv(csd_pv_map[i].ctd_id, size, buf)) != CSD_NO_ERROR) {
		return status;
	}

	memcpy(pxPvValue, buf, xPvLength);
	CSD_DUMP("pxPvValue", pxPvValue, xPvLength);

	CSD_INFO("\n");
	return CSD_NO_ERROR;
}

TCsdStatus csdSetScsTotalAreaSize(TCsdScsTotalAreaSize xScsTotalAreaSize)
{
	CSD_INFO("\n");
	return CSD_ERROR_OPERATION_NOT_SUPPORTED;
}

TCsdStatus csdGetScsTotalAreaSize(TCsdScsTotalAreaSize * pxTotalAreaSize)
{
	CSD_INFO("\n");
	return CSD_ERROR_OPERATION_NOT_SUPPORTED;
}

TCsdStatus csdSetRamUnscrambledStorageSize(
	TCsdUnsignedInt8 *
	pxPvRamUnscrambledStorageSize,
	TCsdUnsignedInt16 xPvLength)
{
	CSD_INFO("\n");
	return CSD_ERROR_OPERATION_NOT_SUPPORTED;
}

TCsdStatus csdGetRamUnscrambledStorageSize(
	TCsdUnsignedInt8 *
	pxPvRamUnscrambledStorageSize,
	TCsdUnsignedInt16 xPvLength)
{
	CSD_INFO("\n");
	return CSD_ERROR_OPERATION_NOT_SUPPORTED;
}

TCsdStatus csdEnableRamScrambling(void)
{
	CSD_INFO("\n");
	return CSD_ERROR_OPERATION_NOT_SUPPORTED;
}

TCsdStatus csdGetScramblingActivationFlag(
	TCsdActivationFlag *
	pxScramblingActivationFlagState)
{
	TCsdBoolean enabled = FALSE;
	TCsdStatus status = CSD_ERROR;
	mt_u32 flag = RAM_SCRAMBLE_ACTIVATION_DISABLED;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(pxScramblingActivationFlagState != NULL,
		CSD_ERROR_INVALID_PARAMETERS);

	if (CSD_NO_ERROR != mt_flash_read_bulk(RAM_SCRAMBLE_ACTIVATION_ADDR,
		RAM_SCRAMBLE_ACTIVATION_SIZE, (mt_u8 *) & flag)) {
		status = CSD_ERROR;
		goto OUT;
	}

	status = CSD_NO_ERROR;
OUT:
	//mt_flash_close(handle);
	enabled = (flag == RAM_SCRAMBLE_ACTIVATION_ENABLED);
	if (enabled)
		*pxScramblingActivationFlagState = CSD_ACTIVATION_FLAG_SET;
	else
		*pxScramblingActivationFlagState = CSD_ACTIVATION_FLAG_NOT_SET;

	CSD_INFO("*pxScramblingActivationFlagState = %d\n",
		 *pxScramblingActivationFlagState);
	return status;
}

TCsdStatus csdEnableFlashProtection(void)
{
	CSD_INFO("\n");
	return CSD_ERROR_OPERATION_NOT_SUPPORTED;
}

TCsdStatus csdGetFlashProtectionActivationFlag(
	TCsdActivationFlag *pxFlashProtectionActivationFlagState)
{
	mt_u32 flag = FLASH_PROTECTION_ACTIVATION_DISABLED;
	TCsdStatus status = CSD_ERROR;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(pxFlashProtectionActivationFlagState != NULL,
		CSD_ERROR_INVALID_PARAMETERS);

	if (CSD_NO_ERROR != mt_flash_read_bulk(FLASH_PROTECT_ACTIVATION_ADDR,
		FLASH_PROTECT_ACTIVATION_SIZE, (mt_u8 *) & flag)) {
		status = CSD_ERROR;
		goto OUT;
	}
	status = CSD_NO_ERROR;
      OUT:

	*pxFlashProtectionActivationFlagState = (flag ==
		RAM_SCRAMBLE_ACTIVATION_ENABLED) ? CSD_ACTIVATION_FLAG_SET :
		CSD_ACTIVATION_FLAG_NOT_SET;
	CSD_INFO("*pxFlashProtectionActivationFlagState = %d\n", *pxFlashProtectionActivationFlagState);
	return status;
}

TCsdStatus csdSelectDebugInterfaceProtectionLevel(
	TCsdDebugInterfaceAccessMode xDebugInterfaceProtectionLevel)
{
	TCsdDebugInterfaceAccessMode cur_level;
	MT_U32 data = 0, reeJtag = 0;
	TCsdStatus status = CSD_ERROR;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	CSD_INFO("xDebugInterfaceProtectionLevel = %d\n", xDebugInterfaceProtectionLevel);

	if (csdGetDebugInterfaceProtectionLevel(&cur_level) != CSD_NO_ERROR)
		return CSD_ERROR;

	RETURN_VAL_IF_FAIL(cur_level <= xDebugInterfaceProtectionLevel,
		CSD_ERROR_OPERATION_NOT_ALLOWED);

	switch (xDebugInterfaceProtectionLevel) {
	case CSD_DEBUG_INTERFACE_ACCESS_MODE_OPEN:
		data = I2C_MODE_OPEN;
		reeJtag = REEJTAG_MODE_OPEN;
		break;
	case CSD_DEBUG_INTERFACE_ACCESS_MODE_PASSWORD_PROTECTED:
		data = I2C_MODE_PWD;
		reeJtag = REEJTAG_MODE_PWD;
		break;
	case CSD_DEBUG_INTERFACE_ACCESS_MODE_CLOSED:
		data = I2C_MODE_CLOSED;
		reeJtag = REEJTAG_MODE_CLOSED;
		break;
	default:
		return CSD_ERROR_INVALID_PARAMETERS;
		break;
	}

	if (mt_otp_write_byte(OTP_REEJTAG_MODE_OFFSET, OTP_REEJTAG_MODE_SIZE,
		(MT_U8 *) & reeJtag) != MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}
	if (mt_otp_write_byte(OTP_I2C_MODE_OFFSET, OTP_I2C_MODE_SIZE,
		(MT_U8 *) & data) != MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}


	status = CSD_NO_ERROR;
      OUT:
	CSD_INFO("status = %d\n", status);
	return status;

}

TCsdStatus csdGetDebugInterfaceProtectionLevel(
	TCsdDebugInterfaceAccessMode *pxDebugInterfaceProtectionLevel)
{
	MT_U32 data = 0;
	TCsdStatus status = CSD_ERROR;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(pxDebugInterfaceProtectionLevel != NULL,
		CSD_ERROR_INVALID_PARAMETERS);

	if (mt_otp_read_byte(OTP_I2C_MODE_OFFSET, OTP_I2C_MODE_SIZE,
		(MT_U8 *) & data) != MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}
	CSD_INFO("data=0x%x \n", data);
	if (I2C_MODE_IS_CLOSED(data)) {
		*pxDebugInterfaceProtectionLevel = CSD_DEBUG_INTERFACE_ACCESS_MODE_CLOSED;
	} else if (I2C_MODE_IS_PWD(data)) {
		*pxDebugInterfaceProtectionLevel = CSD_DEBUG_INTERFACE_ACCESS_MODE_PASSWORD_PROTECTED;
	} else if (I2C_MODE_IS_OPEN(data)) {
		*pxDebugInterfaceProtectionLevel = CSD_DEBUG_INTERFACE_ACCESS_MODE_OPEN;
	} else {
		status = CSD_ERROR;
		goto OUT;
	}

	status = CSD_NO_ERROR;
OUT:
	CSD_INFO("*pxDebugInterfaceProtectionLevel = %d\n", *pxDebugInterfaceProtectionLevel);
	return status;
}

TCsdStatus csdDeactivateKeyLadder(void)
{
	TCsdStatus status = CSD_ERROR;
	MT_U16 value = 0;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);

	value = OTP_DEACTIVE_SCK_VALUE;
	if (mt_otp_write_byte(OTP_DEACTIVE_SCK_OFFSET, OTP_DEACTIVE_SCK_SIZE,
		(MT_U8 *) & value) != MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}

	status = CSD_NO_ERROR;
      OUT:
	CSD_INFO("status = %d\n", status);
	return status;
}

TCsdStatus csdGetKeyLadderDeactivationFlag(
	TCsdActivationFlag *pxKeyLadderDeactivationFlagState)
{
	TCsdStatus status = CSD_ERROR;
	MT_U16 value = 0;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(pxKeyLadderDeactivationFlagState != NULL,
		CSD_ERROR_INVALID_PARAMETERS);

	if (mt_otp_read_byte(OTP_DEACTIVE_SCK_OFFSET, OTP_DEACTIVE_SCK_SIZE,
		(MT_U8 *) & value) != MT_SUCCESS) {
		status = CSD_ERROR;
		goto OUT;
	}

	*pxKeyLadderDeactivationFlagState =
		(value == OTP_DEACTIVE_SCK_VALUE) ? CSD_ACTIVATION_FLAG_SET :
		CSD_ACTIVATION_FLAG_NOT_SET;
	status = CSD_NO_ERROR;
      OUT:
	CSD_INFO("*pxKeyLadderDeactivationFlagState = %d, status = %d\n",
		*pxKeyLadderDeactivationFlagState, status);
	return status;
}

TCsdStatus csdGetDvrKey(TCsdUnsignedInt8 xDvrKey[16])
{
	static mt_u8 cipher[] = { 0xbf, 0x18, 0x6a, 0x73, 0xec, 0x86, 0x3f, 0x25, 0x9b, 0xe8, 0x03,
		0x52, 0x40, 0xd7, 0x37, 0x8c
	};

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xDvrKey != NULL, CSD_ERROR_INVALID_PARAMETERS);

	if (csdDecryptDataWithSecretContentKey(CSD_R2R_ALGORITHM_TDES_K1K2K1,
		CSD_R2R_CRYPTO_OPERATION_MODE_ECB,
		kl_prot_keys,
		(const TCsdUnsignedInt8 *)
		kl_prot_keys, 16, NULL, 0, TRUE,
		NULL, cipher, xDvrKey,
		16) != CSD_NO_ERROR)
		return CSD_ERROR;

	return CSD_NO_ERROR;
}

static TCsdStatus mt_csd_cipher_params(
	TCsdR2RAlgorithm xAlgorithm,
	TCsdR2RCryptoOperationMode xMode,
	CIPHER_KEY_MODE key_mode,
	MT_CIPHER_CTRL_S * p_s_ctrl,
	MT_CIPHER_KEYLADDER_SOURCE_E * p_sck)
{
	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(p_s_ctrl != NULL, CSD_ERROR_INVALID_PARAMETERS);
	if (key_mode == CIPHER_SECRET_KEY)
		RETURN_VAL_IF_FAIL(p_sck != NULL, CSD_ERROR_INVALID_PARAMETERS);

	if (xAlgorithm == CSD_R2R_ALGORITHM_AES_128) {
		p_s_ctrl->algorithm = MT_CIPHER_ALG_AES;

		if (key_mode == CIPHER_SECRET_KEY)
			*p_sck = NOCS_AES_ROOTKEY;
	} else if (xAlgorithm == CSD_R2R_ALGORITHM_TDES_K1K2K1) {
		p_s_ctrl->algorithm = MT_CIPHER_ALG_TDES;

		if (key_mode == CIPHER_SECRET_KEY)
			*p_sck = NOCS_TDES_ROOTKEY;
	} else {
		return CSD_ERROR_OPERATION_NOT_SUPPORTED;
	}

	if (xMode == CSD_R2R_CRYPTO_OPERATION_MODE_CBC) {
		p_s_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
	} else if (xMode == CSD_R2R_CRYPTO_OPERATION_MODE_ECB) {
		p_s_ctrl->work_mode = MT_CIPHER_WORK_MODE_ECB;
	} else {
		return CSD_ERROR_OPERATION_NOT_SUPPORTED;
	}

	return CSD_NO_ERROR;
}

/******************************************************************************/
/*                                                                            */
/*                       Legacy RAM2RAM Functions                             */
/*                                                                            */
/******************************************************************************/

TCsdStatus csdEncryptDataWithClearTextHostKey(
	TCsdR2RAlgorithm xAlgorithm,
	TCsdR2RCryptoOperationMode xMode,
	const TCsdUnsignedInt8 *pxClearTextHostKey,
	TCsdSize xClearTextHostKeySize,
	const TCsdUnsignedInt8 *pxInitializationVector,
	TCsdSize xInitializationVectorSize,
	TCsdBoolean xRefreshIv,
	TCsdR2RKeyPathHandle *pxR2RKeyPathHandle,
	const TCsdUnsignedInt8 *pxInputData,
	TCsdUnsignedInt8 * pxOutputData,
	TCsdSize xDataSize)
{
	TCsdStatus csd_ret = CSD_NO_ERROR;
	mt_s32 ret = MT_SUCCESS;
	mt_u32 kt_slot = MT_CIPHER_KEYSLOT_INVALID;
	mt_handle crypto_handle = MT_INVALID_HANDLE;
	MT_CIPHER_CTRL_S s_ctrl;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(pxClearTextHostKey != NULL, CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxInputData != NULL, CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxOutputData != NULL, CSD_ERROR_INVALID_PARAMETERS);

	memset(&s_ctrl, 0x00, sizeof(MT_CIPHER_CTRL_S));
	s_ctrl.core = MT_CIPHER_CORE_M2M_RAW;
	s_ctrl.operation = MT_CIPHER_OPERATION_ENCRYPT;

	csd_ret = mt_csd_cipher_params(xAlgorithm, xMode, CIPHER_CLEAR_KEY, &s_ctrl, NULL);
	RETURN_VAL_IF_FAIL(csd_ret == CSD_NO_ERROR, csd_ret);

	if (s_ctrl.algorithm == MT_CIPHER_ALG_AES) {
		RETURN_VAL_IF_FAIL(xClearTextHostKeySize == 16, CSD_ERROR_INVALID_PARAMETERS);
		RETURN_VAL_IF_FAIL((xDataSize % 16) == 0, CSD_ERROR_INVALID_PARAMETERS);
	} else if (s_ctrl.algorithm == MT_CIPHER_ALG_TDES) {
		RETURN_VAL_IF_FAIL(xClearTextHostKeySize == 16, CSD_ERROR_INVALID_PARAMETERS);
		RETURN_VAL_IF_FAIL((xDataSize % 8) == 0, CSD_ERROR_INVALID_PARAMETERS);
	}

	//CSD_DUMP("pxClearTextHostKey", pxClearTextHostKey, xClearTextHostKeySize);
	//CSD_DUMP("pxInputData", pxInputData, xDataSize);

	ret = mt_unf_cipher_keyslot_request(&kt_slot);
	if (ret != MT_SUCCESS || kt_slot == MT_CIPHER_KEYSLOT_INVALID) {
		//requested id:0~127
		return CSD_ERROR;
	}

	ret = mt_unf_cipher_keyslot_set(kt_slot, &s_ctrl, (mt_u8 *)pxClearTextHostKey, NULL);
	if (ret != MT_SUCCESS) {
		csd_ret = CSD_ERROR;
		goto EXIT;
	}

	if (xRefreshIv) {
		ret = mt_unf_cipher_keyslot_set_iv(kt_slot, (mt_u8 *)pxInitializationVector,
			xInitializationVectorSize);
		if (ret != MT_SUCCESS) {
			csd_ret = CSD_ERROR;
			goto EXIT;
		}
	}

	ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &crypto_handle);
	if (ret != MT_SUCCESS) {
		csd_ret = CSD_ERROR;
		goto EXIT;
	}

	ret = mt_unf_cipher_crypto_config(crypto_handle, &s_ctrl, kt_slot);
	if (ret != MT_SUCCESS) {
		csd_ret = CSD_ERROR;
		goto EXIT;
	}

	ret = mt_unf_cipher_crypto_process(crypto_handle,
		(mt_u8 *)pxInputData, (mt_u8 *)pxOutputData, 	xDataSize);
	if (ret != MT_SUCCESS) {
		csd_ret = CSD_ERROR;
		goto EXIT;
	}

	//CSD_DUMP("pxOutputData", pxOutputData, xDataSize);

EXIT:
	if (crypto_handle != MT_INVALID_HANDLE)
		mt_unf_cipher_crypto_destroy(crypto_handle);

	mt_unf_cipher_keyslot_release(kt_slot);

	CSD_INFO("\n");
	return csd_ret;
}

TCsdStatus csdDecryptDataWithClearTextHostKey(
	TCsdR2RAlgorithm xAlgorithm,
	TCsdR2RCryptoOperationMode xMode,
	const TCsdUnsignedInt8 *pxClearTextHostKey,
	TCsdSize xClearTextHostKeySize,
	const TCsdUnsignedInt8 *pxInitializationVector,
	TCsdSize xInitializationVectorSize,
	TCsdBoolean xRefreshIv,
	TCsdR2RKeyPathHandle *pxR2RKeyPathHandle,
	const TCsdUnsignedInt8 *pxInputData,
	TCsdUnsignedInt8 * pxOutputData,
	TCsdSize xDataSize)
{
	TCsdStatus csd_ret = CSD_NO_ERROR;
	mt_s32 ret = MT_SUCCESS;
	mt_u32 kt_slot = MT_CIPHER_KEYSLOT_INVALID;
	mt_handle crypto_handle = MT_INVALID_HANDLE;
	MT_CIPHER_CTRL_S s_ctrl;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(pxClearTextHostKey != NULL, CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxInputData != NULL, CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxOutputData != NULL, CSD_ERROR_INVALID_PARAMETERS);

	memset(&s_ctrl, 0x00, sizeof(MT_CIPHER_CTRL_S));
	s_ctrl.core = MT_CIPHER_CORE_M2M_RAW;
	s_ctrl.operation = MT_CIPHER_OPERATION_DECRYPT;

	csd_ret = mt_csd_cipher_params(xAlgorithm, xMode, CIPHER_CLEAR_KEY, &s_ctrl, NULL);
	RETURN_VAL_IF_FAIL(csd_ret == CSD_NO_ERROR, csd_ret);

	if (s_ctrl.algorithm == MT_CIPHER_ALG_AES) {
		RETURN_VAL_IF_FAIL(xClearTextHostKeySize == 16, CSD_ERROR_INVALID_PARAMETERS);
		RETURN_VAL_IF_FAIL((xDataSize % 16) == 0, CSD_ERROR_INVALID_PARAMETERS);
	} else if (s_ctrl.algorithm == MT_CIPHER_ALG_TDES) {
		RETURN_VAL_IF_FAIL(xClearTextHostKeySize == 16, CSD_ERROR_INVALID_PARAMETERS);
		RETURN_VAL_IF_FAIL((xDataSize % 8) == 0, CSD_ERROR_INVALID_PARAMETERS);
	}

	//CSD_DUMP("pxClearTextHostKey", (mt_u8 *)pxClearTextHostKey, xClearTextHostKeySize);
	//CSD_DUMP("pxInputData", (mt_u8 *)pxInputData, xDataSize);

	ret = mt_unf_cipher_keyslot_request(&kt_slot);
	if (ret != MT_SUCCESS || kt_slot == MT_CIPHER_KEYSLOT_INVALID) {
		return CSD_ERROR;
	}

	ret = mt_unf_cipher_keyslot_set(kt_slot, &s_ctrl, (mt_u8 *)pxClearTextHostKey, NULL);
	if (ret != MT_SUCCESS) {
		csd_ret = CSD_ERROR;
		goto EXIT;
	}

	if (xRefreshIv) {
		ret = mt_unf_cipher_keyslot_set_iv(kt_slot, (mt_u8 *)pxInitializationVector,
			xInitializationVectorSize);
		if (ret != MT_SUCCESS) {
			csd_ret = CSD_ERROR;
			goto EXIT;
		}
	}

	ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &crypto_handle);
	if (ret != MT_SUCCESS) {
		csd_ret = CSD_ERROR;
		goto EXIT;
	}

	ret = mt_unf_cipher_crypto_config(crypto_handle, &s_ctrl, kt_slot);
	if (ret != MT_SUCCESS) {
		csd_ret = CSD_ERROR;
		goto EXIT;
	}

	ret = mt_unf_cipher_crypto_process(crypto_handle,
		(mt_u8 *)pxInputData, (mt_u8 *)pxOutputData, 	xDataSize);
	if (ret != MT_SUCCESS) {
		csd_ret = CSD_ERROR;
		goto EXIT;
	}

	//CSD_DUMP("pxOutputData", pxOutputData, xDataSize);

EXIT:
	if (crypto_handle != MT_INVALID_HANDLE)
		mt_unf_cipher_crypto_destroy(crypto_handle);

	mt_unf_cipher_keyslot_release(kt_slot);

	CSD_INFO("\n");
	return csd_ret;
}

TCsdStatus csdEncryptDataWithSecretContentKey(
	TCsdR2RAlgorithm xAlgorithm,
	TCsdR2RCryptoOperationMode xMode,
	const TCsdR2RCipheredProtectingKeys xR2RCipheredProtectingKeys,
	const TCsdUnsignedInt8 *pxCipheredContentKey,
	TCsdSize xCipheredContentKeySize,
	const TCsdUnsignedInt8 *pxInitializationVector,
	TCsdSize xInitializationVectorSize,
	TCsdBoolean xRefreshIv,
	TCsdR2RKeyPathHandle *pxR2RKeyPathHandle,
	const TCsdUnsignedInt8 *pxInputData,
	TCsdUnsignedInt8 * pxOutputData,
	TCsdSize xDataSize)
{
	mt_s32 ret = MT_SUCCESS;
	TCsdStatus csd_ret = CSD_NO_ERROR;
	mt_u32 kt_slot = MT_CIPHER_KEYSLOT_INVALID;
	MT_CIPHER_KEYLADDER_SOURCE_E sck = MT_CIPHER_KEYLADDER_SCK_UNKNOWN;
	mt_handle crypto_handle = MT_INVALID_HANDLE;
	MT_CIPHER_CTRL_S s_ctrl;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xR2RCipheredProtectingKeys != NULL,
		CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxCipheredContentKey != NULL,
		CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxInputData != NULL, CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxOutputData != NULL, CSD_ERROR_INVALID_PARAMETERS);

	memset(&s_ctrl, 0x00, sizeof(MT_CIPHER_CTRL_S));
	s_ctrl.core = MT_CIPHER_CORE_M2M_RAW;
	s_ctrl.operation = MT_CIPHER_OPERATION_ENCRYPT;

	csd_ret = mt_csd_cipher_params(xAlgorithm, xMode, CIPHER_SECRET_KEY, &s_ctrl, &sck);
	RETURN_VAL_IF_FAIL(csd_ret == CSD_NO_ERROR, csd_ret);

	if (s_ctrl.algorithm == MT_CIPHER_ALG_AES) {
		RETURN_VAL_IF_FAIL(xCipheredContentKeySize == 16, CSD_ERROR_INVALID_PARAMETERS);
		RETURN_VAL_IF_FAIL((xDataSize % 16) == 0, CSD_ERROR_INVALID_PARAMETERS);
	} else if (s_ctrl.algorithm == MT_CIPHER_ALG_TDES) {
		RETURN_VAL_IF_FAIL(xCipheredContentKeySize == 16, CSD_ERROR_INVALID_PARAMETERS);
		RETURN_VAL_IF_FAIL((xDataSize % 8) == 0, CSD_ERROR_INVALID_PARAMETERS);
	}

	ret = mt_unf_cipher_keyslot_request(&kt_slot);
	if (ret != MT_SUCCESS || kt_slot == MT_CIPHER_KEYSLOT_INVALID) {
		return CSD_ERROR;
	}

	csd_ret = pcsdKeyLadder(sck, (const mt_u8 *)xR2RCipheredProtectingKeys,
		(const mt_u8 *)xR2RCipheredProtectingKeys + 16,
		(const mt_u8 *)pxCipheredContentKey, kt_slot, MT_CIPHER_ALG_TDES);
	if (csd_ret != CSD_NO_ERROR) {
		goto EXIT;
	}

	if (xRefreshIv && pxInitializationVector) {
		ret = mt_unf_cipher_keyslot_set_iv(kt_slot, (mt_u8 *)pxInitializationVector,
			xInitializationVectorSize);
		if (ret != MT_SUCCESS) {
			csd_ret = CSD_ERROR;
			goto EXIT;
		}
	}

	mt_unf_cipher_keyslot_info(kt_slot);

	ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &crypto_handle);
	if (ret != MT_SUCCESS) {
		csd_ret = CSD_ERROR;
		goto EXIT;
	}

	ret = mt_unf_cipher_crypto_config(crypto_handle, &s_ctrl, kt_slot);
	if (ret != MT_SUCCESS) {
		csd_ret = CSD_ERROR;
		goto EXIT;
	}

	ret = mt_unf_cipher_crypto_process(crypto_handle,
		(mt_u8 *)pxInputData, (mt_u8 *)pxOutputData, 	xDataSize);
	if (ret != MT_SUCCESS) {
		csd_ret = CSD_ERROR;
		goto EXIT;
	}

	CSD_DUMP("pxOutputData", pxOutputData, xDataSize);

EXIT:
	if (crypto_handle != MT_INVALID_HANDLE)
		mt_unf_cipher_crypto_destroy(crypto_handle);

	mt_unf_cipher_keyslot_release(kt_slot);

	CSD_INFO("\n");
	return csd_ret;
}

TCsdStatus csdDecryptDataWithSecretContentKey(
	TCsdR2RAlgorithm xAlgorithm,
	TCsdR2RCryptoOperationMode xMode,
	const
	TCsdR2RCipheredProtectingKeys xR2RCipheredProtectingKeys,
	const TCsdUnsignedInt8 *pxCipheredContentKey,
	TCsdSize xCipheredContentKeySize,
	const TCsdUnsignedInt8 *pxInitializationVector,
	TCsdSize xInitializationVectorSize,
	TCsdBoolean xRefreshIv,
	TCsdR2RKeyPathHandle *pxR2RKeyPathHandle,
	const TCsdUnsignedInt8 *pxInputData,
	TCsdUnsignedInt8 * pxOutputData,
	TCsdSize xDataSize)
{
	mt_s32 ret = MT_SUCCESS;
	TCsdStatus csd_ret = CSD_NO_ERROR;
	mt_u32 kt_slot = MT_CIPHER_KEYSLOT_INVALID;
	MT_CIPHER_KEYLADDER_SOURCE_E sck = MT_CIPHER_KEYLADDER_SCK_UNKNOWN;
	mt_handle crypto_handle = MT_INVALID_HANDLE;
	MT_CIPHER_CTRL_S s_ctrl;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xR2RCipheredProtectingKeys != NULL,
		CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxCipheredContentKey != NULL,
		CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxInputData != NULL, CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxOutputData != NULL, CSD_ERROR_INVALID_PARAMETERS);

	//CSD_INFO("xAlgorithm=0x%x, xMode=0x%x, pxInitializationVector=0x%x, xRefreshIv=0x%x\n",
	//	xAlgorithm, xMode, (mt_u32)pxInitializationVector, xRefreshIv);

	memset(&s_ctrl, 0x00, sizeof(MT_CIPHER_CTRL_S));
	s_ctrl.core = MT_CIPHER_CORE_M2M_RAW;
	s_ctrl.operation = MT_CIPHER_OPERATION_DECRYPT;

	csd_ret = mt_csd_cipher_params(xAlgorithm, xMode, CIPHER_SECRET_KEY, &s_ctrl, &sck);
	RETURN_VAL_IF_FAIL(csd_ret == CSD_NO_ERROR, csd_ret);

	if (s_ctrl.algorithm == MT_CIPHER_ALG_AES) {
		RETURN_VAL_IF_FAIL(xCipheredContentKeySize == 16, CSD_ERROR_INVALID_PARAMETERS);
		RETURN_VAL_IF_FAIL((xDataSize % 16) == 0, CSD_ERROR_INVALID_PARAMETERS);
	} else if (s_ctrl.algorithm == MT_CIPHER_ALG_TDES) {
		RETURN_VAL_IF_FAIL(xCipheredContentKeySize == 16, CSD_ERROR_INVALID_PARAMETERS);
		RETURN_VAL_IF_FAIL((xDataSize % 8) == 0, CSD_ERROR_INVALID_PARAMETERS);
	}

	ret = mt_unf_cipher_keyslot_request(&kt_slot);
	if (ret != MT_SUCCESS || kt_slot == MT_CIPHER_KEYSLOT_INVALID) {
		return CSD_ERROR;
	}

	csd_ret = pcsdKeyLadder(sck, (const mt_u8 *)xR2RCipheredProtectingKeys,
		(const mt_u8 *)xR2RCipheredProtectingKeys + 16,
		(const mt_u8 *)pxCipheredContentKey, kt_slot, MT_CIPHER_ALG_TDES);
	if (csd_ret != CSD_NO_ERROR) {
		goto EXIT;
	}

	if (xRefreshIv && pxInitializationVector) {
		ret = mt_unf_cipher_keyslot_set_iv(kt_slot, (mt_u8 *)pxInitializationVector,
			xInitializationVectorSize);
		if (ret != MT_SUCCESS) {
			csd_ret = CSD_ERROR;
			goto EXIT;
		}
	}

	mt_unf_cipher_keyslot_info(kt_slot);

	ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &crypto_handle);
	if (ret != MT_SUCCESS) {
		csd_ret = CSD_ERROR;
		goto EXIT;
	}

	ret = mt_unf_cipher_crypto_config(crypto_handle, &s_ctrl, kt_slot);
	if (ret != MT_SUCCESS) {
		csd_ret = CSD_ERROR;
		goto EXIT;
	}

	ret = mt_unf_cipher_crypto_process(crypto_handle,
		(mt_u8 *)pxInputData, (mt_u8 *)pxOutputData, 	xDataSize);
	if (ret != MT_SUCCESS) {
		csd_ret = CSD_ERROR;
		goto EXIT;
	}

	//CSD_DUMP("pxOutputData", pxOutputData, xDataSize);

EXIT:
	if (crypto_handle != MT_INVALID_HANDLE)
		mt_unf_cipher_crypto_destroy(crypto_handle);

	mt_unf_cipher_keyslot_release(kt_slot);
	return csd_ret;
}

/******************************************************************************/
/*                                                                            */
/*                                    DSC                                     */
/*                                                                            */
/******************************************************************************/

TCsdStatus csdSetClearTextDscHostKeys(
	TCsdUnsignedInt16 xEmi,
	const TCsdUnsignedInt8 *pxClearTextDscOddHostKey,
	TCsdSize xClearTextDscOddHostKeySize,
	const TCsdUnsignedInt8 *pxClearTextDscEvenHostKey,
	TCsdSize xClearTextDscEvenHostKeySize,
	TCsdDscKeyPathHandle * pxDscKeyPathHandle)
{
	MT_CIPHER_CTRL_S s_ctrl;
	int ret = 0;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL((pxClearTextDscOddHostKey != NULL)
		|| (pxClearTextDscEvenHostKey != NULL),
		CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxDscKeyPathHandle != NULL,
		CSD_ERROR_INVALID_KEY_PATH_HANDLE);

	CSD_DUMP("pxClearTextDscOddHostKey",
		(mt_u8 *)pxClearTextDscOddHostKey, xClearTextDscOddHostKeySize);
	CSD_DUMP("pxClearTextDscEvenHostKey",
		(mt_u8 *)pxClearTextDscEvenHostKey, xClearTextDscEvenHostKeySize);

	RETURN_VAL_IF_FAIL(pcsdChkEmiRange(xEmi),
		CSD_ERROR_OPERATION_NOT_SUPPORTED);
	RETURN_VAL_IF_FAIL(pcsdChkKeySize(xEmi, xClearTextDscOddHostKeySize),
		CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pcsdChkKeySize(xEmi, xClearTextDscEvenHostKeySize),
		CSD_ERROR_INVALID_PARAMETERS);

	if (pxDscKeyPathHandle->iv_len > 0) {
		CSD_DUMP("iv", pxDscKeyPathHandle->iv, pxDscKeyPathHandle->iv_len);
		RETURN_VAL_IF_FAIL(pcsdChkIVSize(xEmi, pxDscKeyPathHandle->iv_len),
			CSD_ERROR_INVALID_PARAMETERS);
	}

	memset(&s_ctrl, 0x00, sizeof(MT_CIPHER_CTRL_S));
	RETURN_VAL_IF_FAIL(pcsdSession2KeyCtrl(0, xEmi, &s_ctrl),
		CSD_ERROR_INVALID_PARAMETERS);

	if (pxClearTextDscOddHostKey != NULL) {
		if (pxDscKeyPathHandle->iv_len > 0) {
			ret = mt_unf_cipher_keyslot_set(pxDscKeyPathHandle->odd_slot, &s_ctrl,
					(mt_u8 *)pxClearTextDscOddHostKey, pxDscKeyPathHandle->iv);
			if (ret != 0) {
				return CSD_ERROR;
			}
		} else {
			ret = mt_unf_cipher_keyslot_set(pxDscKeyPathHandle->odd_slot, &s_ctrl,
					(mt_u8 *)pxClearTextDscOddHostKey, NULL);
			if (ret != 0) {
				return CSD_ERROR;
			}
		}
		mt_unf_cipher_keyslot_info(pxDscKeyPathHandle->odd_slot);
	}
	if (pxClearTextDscEvenHostKey != NULL) {
		if (pxDscKeyPathHandle->iv_len > 0) {
			ret = mt_unf_cipher_keyslot_set(pxDscKeyPathHandle->even_slot, &s_ctrl,
					(mt_u8 *)pxClearTextDscEvenHostKey, pxDscKeyPathHandle->iv);
			if (ret != 0) {
				return CSD_ERROR;
			}
		} else {
			ret = mt_unf_cipher_keyslot_set(pxDscKeyPathHandle->even_slot, &s_ctrl,
					(mt_u8 *) pxClearTextDscEvenHostKey, NULL);
			if (ret != 0) {
				return CSD_ERROR;
			}
		}
		mt_unf_cipher_keyslot_info(pxDscKeyPathHandle->even_slot);
	}
	//CSD_INFO("\n");

	return CSD_NO_ERROR;
}

TCsdStatus csdSetProtectedDscContentKeys(
	TCsdUnsignedInt16 xEmi,
	const TCsdDscCipheredProtectingKeys xDscCipheredProtectingKeys,
	const TCsdUnsignedInt8 *pxCipheredDscOddContentKey,
	TCsdSize xCipheredDscOddContentKeySize,
	const TCsdUnsignedInt8 *pxCipheredDscEvenContentKey,
	TCsdSize xCipheredDscEvenContentKeySize,
	TCsdDscKeyPathHandle *pxDscKeyPathHandle)
{
	mt_u32 error;
	MT_CIPHER_KEYLADDER_SOURCE_E sck = pcsdGetRootKeyID(xEmi);
	TUnsignedInt8 CipheredContentKey[16] = {0};

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL(xDscCipheredProtectingKeys != NULL,
		CSD_ERROR_INVALID_KEY_PATH_HANDLE);
	RETURN_VAL_IF_FAIL((pxCipheredDscOddContentKey != NULL)
		|| (pxCipheredDscEvenContentKey != NULL),
		CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxDscKeyPathHandle != NULL,
		CSD_ERROR_INVALID_KEY_PATH_HANDLE);

	CSD_DUMP("pxCipheredDscOddContentKey", (mt_u8 *)pxCipheredDscOddContentKey,
		xCipheredDscOddContentKeySize);
	CSD_DUMP("pxCipheredDscEvenContentKey", (mt_u8 *)pxCipheredDscEvenContentKey,
		xCipheredDscEvenContentKeySize);

	RETURN_VAL_IF_FAIL(pcsdChkEmiRange(xEmi),
		CSD_ERROR_OPERATION_NOT_SUPPORTED);
	RETURN_VAL_IF_FAIL(pcsdChkKeySize(xEmi, xCipheredDscOddContentKeySize),
		CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pcsdChkKeySize(xEmi, xCipheredDscEvenContentKeySize),
		CSD_ERROR_INVALID_PARAMETERS);

	if (pxDscKeyPathHandle->iv_len > 0) {
		CSD_DUMP("iv", pxDscKeyPathHandle->iv, pxDscKeyPathHandle->iv_len);
		RETURN_VAL_IF_FAIL(pcsdChkIVSize(xEmi, pxDscKeyPathHandle->iv_len),
			CSD_ERROR_INVALID_PARAMETERS);
	}

	if (pxCipheredDscOddContentKey != NULL) {
		memset(CipheredContentKey, 0, 16);
		if (xCipheredDscOddContentKeySize == 8) {
			memcpy(CipheredContentKey+8, pxCipheredDscOddContentKey, 8);
		} else {
			memcpy(CipheredContentKey, pxCipheredDscOddContentKey, 16);
		}

		error = pcsdKeyLadder(sck, (const mt_u8 *)xDscCipheredProtectingKeys,
			(const mt_u8 *)xDscCipheredProtectingKeys + 16,
			(const mt_u8 *)CipheredContentKey,
			pxDscKeyPathHandle->odd_slot, MT_CIPHER_ALG_TDES);
		if (error) {
			return CSD_ERROR;
		}

		if (pxDscKeyPathHandle->iv_len > 0)
	        	mt_unf_cipher_keyslot_set_iv(pxDscKeyPathHandle->odd_slot,
				pxDscKeyPathHandle->iv, pxDscKeyPathHandle->iv_len);

		mt_unf_cipher_keyslot_info(pxDscKeyPathHandle->odd_slot);
	}

	if (pxCipheredDscEvenContentKey != NULL) {
		memset(CipheredContentKey, 0, 16);
		if (xCipheredDscEvenContentKeySize == 8) {
			memcpy(CipheredContentKey+8, pxCipheredDscEvenContentKey, 8);
		} else {
			memcpy(CipheredContentKey, pxCipheredDscEvenContentKey, 16);
		}

		error = pcsdKeyLadder(sck, (const mt_u8 *)xDscCipheredProtectingKeys,
			(const mt_u8 *)xDscCipheredProtectingKeys + 16,
			(const mt_u8 *)CipheredContentKey,
			pxDscKeyPathHandle->even_slot, MT_CIPHER_ALG_TDES);
		if (error) {
			return CSD_ERROR;
		}

		if (pxDscKeyPathHandle->iv_len > 0)
	        	mt_unf_cipher_keyslot_set_iv(pxDscKeyPathHandle->even_slot,
				pxDscKeyPathHandle->iv, pxDscKeyPathHandle->iv_len);

		mt_unf_cipher_keyslot_info(pxDscKeyPathHandle->even_slot);
	}

	return CSD_NO_ERROR;
}

TCsdStatus csdSetClearTextScrEncryptionKey(
	TCsdUnsignedInt16 xEmi,
	const TCsdUnsignedInt8 *pxClearTextScrHostKey,
	TCsdSize xClearTextScrHostKeySize,
	const TCsdUnsignedInt8 *pxInitializationVector,
	TCsdSize xInitializationVectorSize,
	TCsdScrKeyPathHandle *pxScrKeyPathHandle)
{
	MT_CIPHER_CTRL_S s_ctrl;
	int ret = 0;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL((pxClearTextScrHostKey != NULL)
		&& (xClearTextScrHostKeySize == 16),
		CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxScrKeyPathHandle != NULL,
		CSD_ERROR_INVALID_KEY_PATH_HANDLE);

	CSD_INFO("xEmi = %x\n", xEmi);
	CSD_DUMP("pxClearTextScrHostKey", (mt_u8 *)pxClearTextScrHostKey,
		xClearTextScrHostKeySize);
	RETURN_VAL_IF_FAIL(pcsdChkEmiRange(xEmi),
		CSD_ERROR_OPERATION_NOT_SUPPORTED);
	RETURN_VAL_IF_FAIL(pcsdChkKeySize(xEmi, xClearTextScrHostKeySize),
		CSD_ERROR_INVALID_PARAMETERS);
	if (NULL != pxInitializationVector) {
		CSD_DUMP("pxInitializationVector", (mt_u8 *)pxInitializationVector,
			xInitializationVectorSize);
		RETURN_VAL_IF_FAIL(pcsdChkIVSize(xEmi, xInitializationVectorSize),
			CSD_ERROR_INVALID_PARAMETERS);
	}

	memset(&s_ctrl, 0x00, sizeof(MT_CIPHER_CTRL_S));
	RETURN_VAL_IF_FAIL(pcsdSession2KeyCtrl(1, xEmi, &s_ctrl),
		CSD_ERROR_INVALID_PARAMETERS);

	ret = mt_unf_cipher_keyslot_set(pxScrKeyPathHandle->even_slot, &s_ctrl,
		(mt_u8 *)pxClearTextScrHostKey, (mt_u8 *)pxInitializationVector);
	if (ret != 0) {
		return CSD_ERROR;
	}
	mt_unf_cipher_keyslot_info(pxScrKeyPathHandle->even_slot);

	return CSD_NO_ERROR;
}

TCsdStatus csdSetClearTextScrDecryptionKey(
	TCsdUnsignedInt16 xEmi,
	const TCsdUnsignedInt8 *pxClearTextScrHostKey,
	TCsdSize xClearTextScrHostKeySize,
	const TCsdUnsignedInt8 *pxInitializationVector,
	TCsdSize xInitializationVectorSize,
	TCsdScrKeyPathHandle *pxScrKeyPathHandle)
{
	MT_CIPHER_CTRL_S s_ctrl;
	int ret = 0;

	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL((pxClearTextScrHostKey != NULL)
		&& (xClearTextScrHostKeySize == 16),
		CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxScrKeyPathHandle != NULL,
		CSD_ERROR_INVALID_KEY_PATH_HANDLE);

	CSD_INFO("xEmi = %x\n", xEmi);
	CSD_DUMP("pxClearTextScrHostKey", (mt_u8 *)pxClearTextScrHostKey,
		xClearTextScrHostKeySize);
	RETURN_VAL_IF_FAIL(pcsdChkEmiRange(xEmi),
		CSD_ERROR_OPERATION_NOT_SUPPORTED);
	RETURN_VAL_IF_FAIL(pcsdChkKeySize(xEmi, xClearTextScrHostKeySize),
		CSD_ERROR_INVALID_PARAMETERS);
	if (NULL != pxInitializationVector) {
		CSD_DUMP("pxInitializationVector", (mt_u8 *)pxInitializationVector,
			xInitializationVectorSize);
		RETURN_VAL_IF_FAIL(pcsdChkIVSize(xEmi, xInitializationVectorSize),
			CSD_ERROR_INVALID_PARAMETERS);
	}

	memset(&s_ctrl, 0x00, sizeof(MT_CIPHER_CTRL_S));
	RETURN_VAL_IF_FAIL(pcsdSession2KeyCtrl(0, xEmi, &s_ctrl),
		CSD_ERROR_INVALID_PARAMETERS);

	ret = mt_unf_cipher_keyslot_set(pxScrKeyPathHandle->even_slot, &s_ctrl,
		(mt_u8 *)pxClearTextScrHostKey, (mt_u8 *)pxInitializationVector);
	if (ret != 0) {
		return CSD_ERROR;
	}
	mt_unf_cipher_keyslot_info(pxScrKeyPathHandle->even_slot);

	return CSD_NO_ERROR;
}

TCsdStatus csdSetProtectedScrDecryptionKey(
	TCsdUnsignedInt16 xEmi,
	const TCsdScrCipheredProtectingKeys xScrCipheredProtectingKeys,
	const TCsdUnsignedInt8 *pxCipheredScrContentKey,
	TCsdSize xCipheredScrContentKeySize,
	const TCsdUnsignedInt8 *pxInitializationVector,
	TCsdSize xInitializationVectorSize,
	TCsdScrKeyPathHandle *pxScrKeyPathHandle)
{
	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL((xScrCipheredProtectingKeys != NULL)
		|| (pxCipheredScrContentKey != NULL),
		CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxScrKeyPathHandle != NULL,
		CSD_ERROR_INVALID_KEY_PATH_HANDLE);
	RETURN_VAL_IF_FAIL(xCipheredScrContentKeySize != 16,
		CSD_ERROR_INVALID_PARAMETERS);

	CSD_INFO("xEmi = %d\n", xEmi);
	CSD_DUMP("xScrCipheredProtectingKeys", (mt_u8 *)xScrCipheredProtectingKeys,
		xCipheredScrContentKeySize * 2);
	CSD_DUMP("pxCipheredScrContentKey", (mt_u8 *)pxCipheredScrContentKey,
		xCipheredScrContentKeySize);

	RETURN_VAL_IF_FAIL(pcsdChkEmiRange(xEmi),
		CSD_ERROR_OPERATION_NOT_SUPPORTED);
	if (NULL != pxInitializationVector) {
		CSD_DUMP("pxInitializationVector", (mt_u8 *)pxInitializationVector,
			xInitializationVectorSize);
		RETURN_VAL_IF_FAIL(pcsdChkIVSize (xEmi, xInitializationVectorSize),
			CSD_ERROR_INVALID_PARAMETERS);
	}

	if (pcsdKeyLadder(pcsdGetRootKeyID(xEmi),
		(const mt_u8 *)xScrCipheredProtectingKeys,
		(const mt_u8 *)xScrCipheredProtectingKeys + 16,
		(const mt_u8 *)pxCipheredScrContentKey,
		pxScrKeyPathHandle->even_slot, MT_CIPHER_ALG_TDES)) {
		return CSD_ERROR;
	}

	if (NULL != pxInitializationVector) {
		if (mt_unf_cipher_keyslot_set_iv(pxScrKeyPathHandle->even_slot,
			(mt_u8 *)pxInitializationVector, xInitializationVectorSize) != MT_SUCCESS)
			return CSD_NO_ERROR;
	}
	mt_unf_cipher_keyslot_info(pxScrKeyPathHandle->even_slot);

	return CSD_NO_ERROR;
}

TCsdStatus csdSetProtectedScrEncryptionKey(
	TCsdUnsignedInt16 xEmi,
	const TCsdScrCipheredProtectingKeys xScrCipheredProtectingKeys,
	const TCsdUnsignedInt8 *pxCipheredScrContentKey,
	TCsdSize xCipheredScrContentKeySize,
	const TCsdUnsignedInt8 *pxInitializationVector,
	TCsdSize xInitializationVectorSize,
	TCsdScrKeyPathHandle *pxScrKeyPathHandle)
{
	CSD_INFO("\n");
	RETURN_VAL_IF_FAIL(csd_inited, CSD_ERROR);
	RETURN_VAL_IF_FAIL((xScrCipheredProtectingKeys != NULL)
		|| (pxCipheredScrContentKey != NULL),
		CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(xCipheredScrContentKeySize != 16,
		CSD_ERROR_INVALID_PARAMETERS);
	RETURN_VAL_IF_FAIL(pxScrKeyPathHandle != NULL,
		CSD_ERROR_INVALID_KEY_PATH_HANDLE);

	CSD_INFO("xEmi = %d\n", xEmi);
	CSD_DUMP("xScrCipheredProtectingKeys", (mt_u8 *)xScrCipheredProtectingKeys,
		xCipheredScrContentKeySize * 2);
	CSD_DUMP("pxCipheredScrContentKey", (mt_u8 *)pxCipheredScrContentKey,
		xCipheredScrContentKeySize);

	RETURN_VAL_IF_FAIL(pcsdChkEmiRange(xEmi),
		CSD_ERROR_OPERATION_NOT_SUPPORTED);
	if (NULL != pxInitializationVector) {
		CSD_DUMP("pxInitializationVector", (mt_u8 *)pxInitializationVector,
			xInitializationVectorSize);
		RETURN_VAL_IF_FAIL(pcsdChkIVSize (xEmi, xInitializationVectorSize),
			CSD_ERROR_INVALID_PARAMETERS);
	}

	if (pcsdKeyLadder(pcsdGetRootKeyID(xEmi),
		(const mt_u8 *)xScrCipheredProtectingKeys,
		(const mt_u8 *)xScrCipheredProtectingKeys + 16,
		(const mt_u8 *)pxCipheredScrContentKey,
		pxScrKeyPathHandle->even_slot, MT_CIPHER_ALG_TDES)) {
		return CSD_ERROR;
	}

	if (NULL != pxInitializationVector) {
		if (mt_unf_cipher_keyslot_set_iv(pxScrKeyPathHandle->even_slot,
			(mt_u8 *)pxInitializationVector, xInitializationVectorSize) != MT_SUCCESS)
			return CSD_NO_ERROR;
	}
	mt_unf_cipher_keyslot_info(pxScrKeyPathHandle->even_slot);

	return CSD_NO_ERROR;
}

