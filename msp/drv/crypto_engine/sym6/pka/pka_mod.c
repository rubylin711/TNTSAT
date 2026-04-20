/***************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/***************************************************************************/
#include <linux/delay.h>
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_type.h"
#include "pka_mod.h"
#include "pka_regs.h"

#define PKA_MOD_MAX_LOOP_CNT  (6000000)
#define PKA_MOD_MAX_WORD_SIZE (64)

#define PKA_MOD_RAM (0)
#define PKA_OP1_RAM (1)
#define PKA_OP2_RAM (2)
#define PKA_OUT_RAM (3)

static mt_s32 pka_pow_mod(mt_u32 word_size, const mt_u8 *mod,
			  const mt_u8 *op1, const mt_u8 *op2, mt_u8 *out)
{
	mt_u32 i;
	mt_s32 ret = 0;
	mt_u32 val;
        mt_u32 *p_u32;

	val  = HAL_GET_U32((volatile mt_u32 *)REG_RSA_CTL) | RSA_CTL_LAG_OP_EN | RSA_CTL_RSA_ENDIAN_BIG;
	HAL_PUT_U32((volatile mt_u32 *)REG_RSA_CTL, val);

	/* Modulus */
        p_u32 = (mt_u32 *)(mod);
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, (PKA_MOD_MAX_WORD_SIZE << 8) | PKA_MOD_RAM);
	for (i = 0; i < PKA_MOD_MAX_WORD_SIZE - word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, 0);
	}
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	/* Base */
        p_u32 = (mt_u32 *)(op1);
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, (word_size << 8) | PKA_OP1_RAM);
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	/* Exponent */
        p_u32 = (mt_u32 *)(op2);
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, (word_size << 8) | PKA_OP2_RAM);
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	val = (1 << 28) | (PKA_ECC_CMD_MOD_POW << 24)
		| (PKA_OUT_RAM << 16) | (PKA_OP2_RAM << 8) | PKA_OP1_RAM;
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_CMD, val);

	while (1) {
                i++;
                if (i % 100 == 0)
                    usleep_range(100, 1000);

#ifdef PKA_MOD_MAX_LOOP_CNT
		if (i >= PKA_MOD_MAX_LOOP_CNT) {
			printk("%s timeout!\n", __FUNCTION__);
			ret = -1;
			goto exit;
		}
#endif
		if ((HAL_GET_U32((volatile mt_u32 *)REG_ECC_CMD) & 0x80000000) == 0)
			break;
	}

        p_u32 = (mt_u32 *)(out);
	for (i = 0; i < word_size; i++) {
		p_u32[i] = HAL_GET_U32((volatile mt_u32 *)REG_ECC_DAT_OUT);
	}

#ifdef PKA_MOD_MAX_LOOP_CNT
exit:
#endif
	val = HAL_GET_U32((volatile mt_u32 *)REG_RSA_CTL)
		& ~(RSA_CTL_LAG_OP_EN | RSA_CTL_RSA_ENDIAN_BIG);
	HAL_PUT_U32((volatile mt_u32 *)REG_RSA_CTL, val);
	return ret;
}

static mt_s32 pka_unary_op_mod(mt_u32 type, mt_u32 word_size, const mt_u8 *mod,
			       const mt_u8 *op, mt_u8 *out)
{
	mt_u32 i;
	mt_s32 ret = 0;
	mt_u32 val;
        mt_u32 *p_u32;

	val = HAL_GET_U32((volatile mt_u32 *)REG_RSA_CTL) | RSA_CTL_LAG_OP_EN | RSA_CTL_RSA_ENDIAN_BIG;
	HAL_PUT_U32((volatile mt_u32 *)REG_RSA_CTL, val);

	/* Modulus */
        p_u32 = (mt_u32 *)(mod);
	val = (PKA_MOD_MAX_WORD_SIZE << 8) | PKA_MOD_RAM;
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, val);
	for (i = 0; i < PKA_MOD_MAX_WORD_SIZE - word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, 0);
	}
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	/* OP */
        p_u32 = (mt_u32 *)(op);
	val = (word_size << 8) | PKA_OP1_RAM;
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, val);
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	val = (1 << 28) | (type << 24) | (PKA_OUT_RAM << 16) | PKA_OP1_RAM;
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_CMD, val);

	while (1) {
                i++;
                if (i % 100 == 0)
                    usleep_range(100, 1000);

#ifdef PKA_MOD_MAX_LOOP_CNT
		if (i >= PKA_MOD_MAX_LOOP_CNT) {
			printk("%s timeout!\n", __FUNCTION__);
			ret = -1;
			goto exit;
		}
#endif
		if ((HAL_GET_U32((volatile mt_u32 *)REG_ECC_CMD) & 0x80000000) == 0)
			break;
	}

        p_u32 = (mt_u32 *)(out);
	for (i = 0; i < word_size; i++) {
		p_u32[i] = HAL_GET_U32((volatile mt_u32 *)REG_ECC_DAT_OUT);
	}

#ifdef PKA_MOD_MAX_LOOP_CNT
exit:
#endif
	val = HAL_GET_U32((volatile mt_u32 *)REG_RSA_CTL)
		& ~(RSA_CTL_LAG_OP_EN | RSA_CTL_RSA_ENDIAN_BIG);
	HAL_PUT_U32((volatile mt_u32 *)REG_RSA_CTL, val);
	return ret;
}

static mt_s32 pka_binary_op_mod(mt_u32 type, mt_u32 word_size,
				const mt_u8 *mod, const mt_u8 *op1,
				const mt_u8 *op2, mt_u8 *out)
{
	mt_u32 i;
	mt_s32 ret = 0;
	mt_u32 val;
        mt_u32 *p_u32;

	val = HAL_GET_U32((volatile mt_u32 *)REG_RSA_CTL) | RSA_CTL_LAG_OP_EN
		| RSA_CTL_RSA_ENDIAN_BIG;
	HAL_PUT_U32((volatile mt_u32 *)REG_RSA_CTL, val);

	/* Modulus */
        p_u32 = (mt_u32 *)(mod);
	val = (PKA_MOD_MAX_WORD_SIZE << 8) | PKA_MOD_RAM;
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, val);
	for (i = 0; i < PKA_MOD_MAX_WORD_SIZE - word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, 0);
	}
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	/* OP1 */
        p_u32 = (mt_u32 *)(op1);
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, (word_size << 8) | PKA_OP1_RAM);
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	/* OP2 */
        p_u32 = (mt_u32 *)(op2);
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, (word_size << 8) | PKA_OP2_RAM);
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	val = (1 << 28) | (type << 24) | (PKA_OUT_RAM << 16)
		| (PKA_OP2_RAM << 8) | PKA_OP1_RAM;
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_CMD, val);

	while (1) {
                i++;
                if (i % 100 == 0)
                    usleep_range(100, 1000);

#ifdef PKA_MOD_MAX_LOOP_CNT
		if (i >= PKA_MOD_MAX_LOOP_CNT) {
			printk("%s timeout!\n", __FUNCTION__);
			ret = -1;
			goto exit;
		}
#endif
		if ((HAL_GET_U32((volatile mt_u32 *)REG_ECC_CMD) & 0x80000000) == 0)
			break;
	}

        p_u32 = (mt_u32 *)(out);
	for (i = 0; i < word_size; i++) {
		p_u32[i] = HAL_GET_U32((volatile mt_u32 *)REG_ECC_DAT_OUT);
	}

#ifdef PKA_MOD_MAX_LOOP_CNT
exit:
#endif
	val = HAL_GET_U32((volatile mt_u32 *)REG_RSA_CTL)
		& ~(RSA_CTL_LAG_OP_EN | RSA_CTL_RSA_ENDIAN_BIG);
	HAL_PUT_U32((volatile mt_u32 *)REG_RSA_CTL, val);

	return ret;
}

mt_s32 pka_mod_operation(mt_u32 cmd, mt_u32 size, const mt_u8 *mod,
			 const mt_u8 *op1, const mt_u8 *op2, mt_u8 *out)
{
	mt_u32 word_size = size >> 2;
	mt_u32 type = 0;

	switch (cmd) {
	case PKA_MOD_CMD_MOD_POW:
		type = PKA_ECC_CMD_MOD_POW;
		break;
	case PKA_MOD_CMD_MOD_MUL:
		type = PKA_ECC_CMD_MOD_MUL;
		break;
	case PKA_MOD_CMD_MOD_ADD:
		type = PKA_ECC_CMD_MOD_ADD;
		break;
	case PKA_MOD_CMD_MOD_SUB:
		type = PKA_ECC_CMD_MOD_SUB;
		break;
	case PKA_MOD_CMD_MOD_INV:
		type = PKA_ECC_CMD_MOD_INV;
		break;
	case PKA_MOD_CMD_MOD:
		type = PKA_ECC_CMD_MOD;
		break;
	default:
		return -1;
		break;
	}

	if (size & 0x3)
		return -1;

	if (word_size > PKA_MOD_MAX_WORD_SIZE)
		return -1;

	if (type == PKA_ECC_CMD_MOD_POW) {
		return pka_pow_mod(word_size, mod, op1, op2, out);
	} else if ((type == PKA_ECC_CMD_MOD) || (type == PKA_ECC_CMD_MOD_INV)) {
		return pka_unary_op_mod(type, word_size, mod, op1, out);
	}

	return pka_binary_op_mod(type, word_size, mod, op1, op2, out);
}

mt_s32 pka_mod_is_even(void)
{
	return (HAL_GET_U32((volatile mt_u32 *)REG_ECC_CMD) & 0x20000000) != 0;
}

void pka_clock_gating_enable(void)
{
	mt_u32 val = HAL_GET_U32((volatile mt_u32 *)REG_RSA_CTL) & ~(7 << 3);
	HAL_PUT_U32((volatile mt_u32 *)REG_RSA_CTL, val);
}

void pka_clock_gating_disable(void)
{
	mt_u32 val = HAL_GET_U32((volatile mt_u32 *)REG_RSA_CTL) | 7 << 3;
	HAL_PUT_U32((volatile mt_u32 *)REG_RSA_CTL, val);
}
