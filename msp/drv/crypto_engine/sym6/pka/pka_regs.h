/***************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/***************************************************************************/
#ifndef _PKA_REGS_H_
#define _PKA_REGS_H_

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#define REG_ECC_BASE    SYMPHONY_IO_VA(0xbf350000)
#define REG_ECC_SIZE    (0x100)

/*
 * RSA_DAT_IN
 * 31:0 RSA_DAT_IN
 */
#define REG_RSA_DAT_IN  (REG_ECC_BASE + 0x00)

/*
 * RSA_DAT_OUT
 * 31:0 RSA_DAT_OUT
 */
#define REG_RSA_DAT_OUT (REG_ECC_BASE + 0x04)

/*
 * RSA_CMD
 * 31:9 RSVD
 *    8 RSA_BUSY
 *  7:2 RSVD
 *  1:0 RSA_TYPE    00: M; 01: E; 1x: X
 */
#define REG_RSA_CMD     (REG_ECC_BASE + 0x08)

/*
 * RSA_CTL
 * 31:8 RSVD
 *    7 LAG_OP_EN           0: Disable large-number operations; 1: Enable large-number operations.
 *    6 RSVD
 *    5 PKA_FUNC_CG_MOD     1: Close PKA FUNCTION clock gating function; 0: Open PKA FUNCTION clock gating function.
 *    4 PKA_GS_CG_MOD       1: Close GS bus clock gating fucntion; 0: Open GS bus clock gating function.
 *    3 PKA_SRAM_CG_MOD     1: Close SRAM and sm2_ahb_top clock gating function; 0: Open SRAM and sm2_ahb_top clock gating function.
 *  2:1 RSVD
 *    0 RSA_ENDIAN          0: Little-endian; 1: Big-endian
 */
#define REG_RSA_CTL     (REG_ECC_BASE + 0x0c)
#define RSA_CTL_LAG_OP_EN      (1 << 7)
#define RSA_CTL_RSA_ENDIAN_BIG (1 << 0)
/*
 * ECC_DAT_IN
 * 31:0 SM2_DAT_IN
 */
#define REG_ECC_DAT_IN  (REG_ECC_BASE + 0x10)

/*
 * ECC_DAT_OUT
 * 31:0 SM2_DAT_OUT
 */
#define REG_ECC_DAT_OUT (REG_ECC_BASE + 0x14)

/*
 * ECC_DAT_CFG
 * 31:16 RSVD
 * 15:8  SM2_LEN
 *  7:4  SM2_ADDR
 *  3:0  SM2_RAM_NUM
 */
#define REG_ECC_DAT_CFG (REG_ECC_BASE + 0x18)

/*
 * ECC_CMD
 * 31    SM2_BUSY
 * 30    MODINV_FAIL
 * 29    EVEN_P_ERR
 * 28    ECC_START
 * 27:24 ECC_CMD
 * 23:20 DST_ADDR
 * 19:16 DST_RAM_NUM
 * 15:12 SRC1_ADDR
 * 11:8  SRC1_RAM_NUM
 *  7:4  SRC0_ADDR
 *  3:0  SRC0_RAM_NUM
 */
#define REG_ECC_CMD     (REG_ECC_BASE + 0x1c)

enum PKA_ECC_CMD_TYPE {
	PKA_ECC_CMD_MOD_POW = 0,
	PKA_ECC_CMD_POINT_MUL,
	PKA_ECC_CMD_POINT_ADD,
	PKA_ECC_CMD_MOD_MUL,
	PKA_ECC_CMD_MOD_ADD,
	PKA_ECC_CMD_MOD_SUB,
	PKA_ECC_CMD_MOD_INV,
	PKA_ECC_CMD_MOD
};

/*
 * SEMPH_CTL
 * 31:4 TIMER_INITIAL
 *  3:0 CPU_ACCESS_CTRL_ID
 */
#define REG_SEMPH_CTL   (REG_ECC_BASE + 0x80)

#endif /* end of include guard: _PKA_REGS_H_ */
