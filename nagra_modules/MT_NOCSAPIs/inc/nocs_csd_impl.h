/*
 * Copyright (C) 2020 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 *
 * This program is confidential and proprietary to Montage Technology Group Limited
 * and its affiliated companies(Montage), and may not be copied, reproduced, modified,
 * disclosed to others, published or used, in whole or in part, without the express
 * prior written permission of Montage.
 */
#ifndef __NOCS_CSD_IMPL_H__
#define __NOCS_CSD_IMPL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nocs_csd.h"

/** @} */  /** <!-- ==== Structure Definition ==== */

/** \addtogroup      NOCS_CSD_IMPL */
/** @{ */  /** <!-- [NOCS_CSD_IMPL] */

//Only used for JTS test
//#define	NAGRA_CSD_JTS_TEST

/**Boot mode of chipset.*/
enum ECsdBootMode {
	CSD_BOOT_SPI_NOR = 0,   /**Boot from SPI-Nor flash. */
	CSD_BOOT_PNAND, /**Boot from RAW-NAND flash. */
	CSD_BOOT_SPI_NAND,  /**Boot from SPI-NAND flash. */
	CSD_BOOT_SPI_EMMC,  /**Boot from SPI-Nor flash. */
	LAST_CSD_BOOT,   /**Invalid boot mode. */
};
#if 0 //It is only used for JTS test, small size can reduce time cost.
	enum ECsdScsSize {
		CSD_SCS_SIZE_64K = 0x10000,
		CSD_SCS_SIZE_128K = 0x20000,
		CSD_SCS_SIZE_256K = 0x40000,
		CSD_SCS_SIZE_512K = 0x80000,
	};
#else
/**The total size of DTE boot code size.*/
    	enum ECsdScsSize {
		CSD_SCS_SIZE_256K = 0x40000,
		CSD_SCS_SIZE_512K = 0x80000,
		CSD_SCS_SIZE_1M = 0x100000,
		CSD_SCS_SIZE_2M = 0x200000,
	};
#endif
/**The opertion mode of the key*/
typedef enum _KeyPathOPMode {
	KeyPathOP_ENC,  /**Encryption operation. */
	KeyPathOP_DEC,  /**Dncryption operation. */
} KeyPathOPMode;

/**It is used for NAGRA CSD API testing*/
struct SCsdScsPvPathHandle {
	unsigned int dummy;	//dummy member to suppress warnings/errors
};
/**It is used for NAGRA CSD API testing*/
struct SCsdInitParameters {
	unsigned int dummy;	//dummy member to suppress warnings/errors
};
/**It is used for NAGRA CSD API testing*/
struct SCsdTerminateParameters {
	unsigned int dummy;	//dummy member to suppress warnings/errors
};
/**The even and odd key slot number for handle*/
struct SCsdScrKeyPathHandle {
	unsigned int even_slot; /**Even key slot number. */
	unsigned int odd_slot;  /**Odd key slot number. */
};
/**The jey information for descrmbler key path*/
typedef struct SCsdDscKeyPathHandle {
	unsigned int dmx_id;	/**The demux ID for descrmbler. */
	unsigned int dsc_handle; /**The descrambler handle. */
	unsigned int even_slot; /**Even key slot number. */
	unsigned int odd_slot;  /**Odd key slot number. */
	unsigned char iv[16];   /**The IV data for key slot. */
	unsigned int iv_len;    /**The IV length for key slot. */
};
/**It is used for NAGRA CSD API testing*/
struct SCsdR2RKeyPathHandle {
	unsigned int dummy;	//dummy member to suppress warnings/errors
}MT_CsdDscKeyPathHandle;

/** @} */  /** <!-- ==== Structure Definition end ==== */

#ifdef __cplusplus
}
#endif
#endif				//__CERT_IP_H__
