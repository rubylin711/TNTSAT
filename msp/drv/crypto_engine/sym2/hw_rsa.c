/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "drv_ce_if.h"
#include "hw_ce_common.h"
#include "hw_ce_register.h"
#include "hw_ce_if.h"


//s32 hw_ce_rsa_process(HW_CE_RSA_CTRL_S *hw_ctrl, mt_u32 *p_src_addr, mt_u32 *p_dst_addr, mt_u32 src_length)
s32 hw_ce_rsa_process(mt_u32 *P, mt_u32 len_P, mt_u32 *E, mt_u32 len_E,
        mt_u32 *src, mt_u32 *dst, mt_u32 src_length)
{
    mt_u32 i, timeout;
    mt_u32 fill_len = 0;
    unsigned int user_bytes = 0;

    HAL_PUT_U32((volatile mt_u32 *)(ECC_ENDIAN), 1);
    HAL_PUT_U32((volatile mt_u32 *)(RSA_CMD), 0);

    if (!P || !E || !src || !dst)
        return CE_RSA_PROCESS_FAILED;

    fill_len = 256 - len_P;
    if (fill_len > 0) {
        /* fill fist 1024 bits 0x0 */
        for (i = 0; i < fill_len/4; i++)
            HAL_PUT_U32((volatile mt_u32 *)(RSA_DAT_IN), 0x0);
    }
    for (i = 0; i < len_P/4; i++) {
        HAL_PUT_U32((volatile mt_u32 *)(RSA_DAT_IN),  P[i]);
    }

    HAL_PUT_U32((volatile mt_u32 *)(RSA_CMD), 1);

    fill_len = 256 - len_E;
    if (fill_len > 0) {
        /* fill fist 1024 bits 0x0 */
        for (i = 0; i < fill_len/4; i++)
            HAL_PUT_U32((volatile mt_u32 *)(RSA_DAT_IN), 0x0);
    }
    for (i = 0; i < len_E/4; i++) {
        HAL_PUT_U32((volatile mt_u32 *)(RSA_DAT_IN), E[i]);
    }

    HAL_PUT_U32((volatile mt_u32 *)(RSA_CMD), 2);

    fill_len = 256 - src_length;
    if (fill_len > 0) {
        /* fill fist 1024 bits 0x0 */
        for (i = 0; i < fill_len/4; i++)
            HAL_PUT_U32((volatile mt_u32 *)(RSA_DAT_IN), 0x0);
    }
    for (i = 0; i < src_length/4; i++) {
        HAL_PUT_U32((volatile mt_u32 *)(RSA_DAT_IN), src[i]);
    }

    timeout = 0;
    while (0x0000100 & HAL_GET_U32((volatile mt_u32 *)(RSA_CMD))) {
        //printk("rsa calc busy now\n");
        hw_ce_msdelay_customize(1);
        timeout++;
        if(timeout > 6000000) {
            printk("[%s::%d]CE_RSA_CALC_TIMEOUT\n", __FUNCTION__, __LINE__);
            return CE_RSA_CALC_TIMEOUT;
        }
    }

    if (fill_len > 0) {
        /* abandon the fist 1024 bits */
        for (i = 0; i < fill_len/4; i++)
            user_bytes = HAL_GET_U32((volatile mt_u32 *)(RSA_DAT_OUT));
    }

    for (i = 0; i < src_length/4; i++) {
        dst[i] = HAL_GET_U32((volatile mt_u32 *)(RSA_DAT_OUT));
    }

    return CE_SUCCESS;
}

void hw_ce_rsa_reset(void)
{
	mt_u32 sw_reset_value = HAL_GET_U32((volatile mt_u32 *)(CRYPTO_SW_RST));

	HAL_PUT_U32((volatile mt_u32 *)(CRYPTO_SW_RST), sw_reset_value & 0xFFFFFFFE);

	hw_ce_msdelay_customize(100);

	HAL_PUT_U32((volatile mt_u32 *)(CRYPTO_SW_RST), sw_reset_value | 0x00000001);
}

