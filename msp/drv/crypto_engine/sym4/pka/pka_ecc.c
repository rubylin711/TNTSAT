/***************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/***************************************************************************/
#include <linux/delay.h>
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_type.h"
#include "pka_ecc.h"
#include "pka_regs.h"

#define PKA_ECC_MAX_LOOP_CNT  (6000000)
#define PKA_ECC_MAX_WORD_SIZE (16)

#define PKA_ECC_P_RAM   (0 << 4 | 0)
#define PKA_ECC_A_RAM   (8 << 4 | 0)
#define PKA_ECC_X1_RAM  (0 << 4 | 1)
#define PKA_ECC_Y1_RAM  (4 << 4 | 1)
#define PKA_ECC_X2_RAM  (0 << 4 | 2)
#define PKA_ECC_Y2_RAM  (4 << 4 | 2)
#define PKA_ECC_K_RAM   (0 << 4 | 2)
#define PKA_ECC_OUT_RAM (0 << 4 | 3)

mt_s32 pka_ecc_point_add(mt_u32 size, const mt_u8 *p, const mt_u8 *a,
			 const mt_u8 *x1, const mt_u8 *y1, const mt_u8 *x2,
			 const mt_u8 *y2, mt_u8 *r_x, mt_u8 *r_y)
{
	mt_u32 i;
	mt_u32 word_size = size >> 2;
	mt_s32 ret = 0;
	mt_u32 val;
        mt_u32 *p_u32;

	if (size & 0x3)
		return -1;

	if (word_size > PKA_ECC_MAX_WORD_SIZE)
		return -1;

	if (!p || !a || !x1 || !y1 || !x2 || !y2 || !r_x || !r_y)
		return -1;

	val = HAL_GET_U32((volatile mt_u32 *)REG_RSA_CTL) | RSA_CTL_RSA_ENDIAN_BIG;
	HAL_PUT_U32((volatile mt_u32 *)REG_RSA_CTL, HAL_GET_U32((volatile mt_u32 *)REG_RSA_CTL) | RSA_CTL_RSA_ENDIAN_BIG);

	/* P */
        p_u32 = (mt_u32 *)(p);
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, (PKA_ECC_MAX_WORD_SIZE << 8) | PKA_ECC_P_RAM);
	for (i = 0; i < PKA_ECC_MAX_WORD_SIZE - word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, 0);
	}
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	/* A */
        p_u32 = (mt_u32 *)(a);
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, (PKA_ECC_MAX_WORD_SIZE << 8) | PKA_ECC_A_RAM);
	for (i = 0; i < PKA_ECC_MAX_WORD_SIZE - word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, 0);
	}
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	/* X1 */
        p_u32 = (mt_u32 *)(x1);
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, (word_size << 8) | PKA_ECC_X1_RAM);
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	/* Y1 */
        p_u32 = (mt_u32 *)(y1);
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, (word_size << 8) | PKA_ECC_Y1_RAM);
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	/* X2 */
        p_u32 = (mt_u32 *)(x2);
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, (word_size << 8) | PKA_ECC_X2_RAM);
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	/* Y2 */
        p_u32 = (mt_u32 *)(y2);
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, (word_size << 8) | PKA_ECC_Y2_RAM);
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	val = (1 << 28) | (PKA_ECC_CMD_POINT_ADD << 24)
	    | (PKA_ECC_OUT_RAM << 16) | (PKA_ECC_X2_RAM << 8)
	    | PKA_ECC_X1_RAM;
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_CMD, val);

	while (1) {
                i++;
                if (i % 100 == 0)
                    usleep_range(100, 1000);

#ifdef PKA_ECC_MAX_LOOP_CNT
		if (i >= PKA_ECC_MAX_LOOP_CNT) {
			printk("%s timeout!\n", __FUNCTION__);
			ret = -1;
			goto exit;
		}
#endif
		if ((HAL_GET_U32((volatile mt_u32 *)REG_ECC_CMD) & 0x80000000) == 0)
			break;
	}

        p_u32 = (mt_u32 *)(r_y);
	for (i = 0; i < word_size; i++) {
		p_u32[i] = HAL_GET_U32((volatile mt_u32 *)REG_ECC_DAT_OUT);
	}
        p_u32 = (mt_u32 *)(r_x);
	for (i = 0; i < word_size; i++) {
		p_u32[i] = HAL_GET_U32((volatile mt_u32 *)REG_ECC_DAT_OUT);
	}

#ifdef PKA_ECC_MAX_LOOP_CNT
exit:
#endif
	return ret;
}

mt_s32 pka_ecc_point_mul(mt_u32 size, const mt_u8 *p, const mt_u8 *a,
			 const mt_u8 *x, const mt_u8 *y, const mt_u8 *k,
			 mt_u8 *r_x, mt_u8 *r_y)
{
	mt_u32 i;
	mt_u32 word_size = size >> 2;
	mt_s32 ret = 0;
	mt_u32 val;
        mt_u32 *p_u32;

	if (size & 0x3)
		return -1;

	if (word_size > PKA_ECC_MAX_WORD_SIZE)
		return -1;

	if (!p || !a || !x || !y || !k || !r_x || !r_y)
		return -1;

	val = HAL_GET_U32((volatile mt_u32 *)REG_RSA_CTL) | RSA_CTL_RSA_ENDIAN_BIG;
	HAL_PUT_U32((volatile mt_u32 *)REG_RSA_CTL, val);

	/* P */
        p_u32 = (mt_u32 *)(p);
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, (PKA_ECC_MAX_WORD_SIZE << 8) | PKA_ECC_P_RAM);
	for (i = 0; i < PKA_ECC_MAX_WORD_SIZE - word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, 0);
	}
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	/* A */
        p_u32 = (mt_u32 *)(a);
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, (PKA_ECC_MAX_WORD_SIZE << 8) | PKA_ECC_A_RAM);
	for (i = 0; i < PKA_ECC_MAX_WORD_SIZE - word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, 0);
	}
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	/* X */
        p_u32 = (mt_u32 *)(x);
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, (word_size << 8) | PKA_ECC_X1_RAM);
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	/* Y */
        p_u32 = (mt_u32 *)(y);
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, (word_size << 8) | PKA_ECC_Y1_RAM);
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	/* K */
        p_u32 = (mt_u32 *)(k);
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_CFG, (word_size << 8) | PKA_ECC_K_RAM);
	for (i = 0; i < word_size; i++) {
		HAL_PUT_U32((volatile mt_u32 *)REG_ECC_DAT_IN, p_u32[i]);
	}

	val = (1 << 28) | (PKA_ECC_CMD_POINT_MUL << 24)
	    | (PKA_ECC_OUT_RAM << 16) | (PKA_ECC_K_RAM << 8)
	    | PKA_ECC_X1_RAM;
	HAL_PUT_U32((volatile mt_u32 *)REG_ECC_CMD, val);

	while (1) {
                i++;
                if (i % 100 == 0)
                    usleep_range(100, 1000);

#ifdef PKA_ECC_MAX_LOOP_CNT
		if (i >= PKA_ECC_MAX_LOOP_CNT) {
			printk("%s timeout!\n", __FUNCTION__);
			ret = -1;
			goto exit;
		}
#endif
		if ((HAL_GET_U32((volatile mt_u32 *)REG_ECC_CMD) & 0x80000000) == 0)
			break;
	}

        p_u32 = (mt_u32 *)(r_y);
	for (i = 0; i < word_size; i++) {
		p_u32[i] = HAL_GET_U32((volatile mt_u32 *)REG_ECC_DAT_OUT);
	}

        p_u32 = (mt_u32 *)(r_x);
	for (i = 0; i < word_size; i++) {
		p_u32[i] = HAL_GET_U32((volatile mt_u32 *)REG_ECC_DAT_OUT);
	}

#ifdef PKA_ECC_MAX_LOOP_CNT
exit:
#endif
	val = HAL_GET_U32((volatile mt_u32 *)REG_RSA_CTL) & ~RSA_CTL_RSA_ENDIAN_BIG;
	HAL_PUT_U32((volatile mt_u32 *)REG_RSA_CTL, val);
	return ret;
}
