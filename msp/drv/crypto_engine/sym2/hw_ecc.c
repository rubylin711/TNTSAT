/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "hw_ce_common.h"
#include "hw_ce_register.h"
#include "hw_ce_if.h"

//a b m 最多256字节长度
s32 hw_bn_mod(HW_BN_METH_E meth, HW_BN_OP_S *op)
{
	u32 i = 0;
	u32 time_out = 0;
	u32 config_ram_id = 0;

	u32 src0_length = op->a_length >> 2;
	u32 src1_length = op->b_length >> 2;
	u32 P_length = op->m_length >> 2;

	u32 *src0 = (u32 *)(op->a);
	u32 *src1 = (u32 *)(op->b);
	u32 *P = (u32 *)(op->m);
	u32 *result = (u32 *)(op->r);
	//u32 temp;

	HAL_PUT_U32((volatile u32 *)(ECC_ENDIAN), 1);

	if(meth == HW_MOD_EXP)
	{
		//temp = HAL_GET_U32((volatile u32 *)(0xBF153004));
		//HAL_PUT_U32((volatile u32 *)(0xBF153004), temp &0xFFFFFFFB);
		//for(i=0; i<65530; i++);
		//temp = HAL_GET_U32((volatile u32 *)(0xBF153004));
		//HAL_PUT_U32((volatile u32 *)(0xBF153004), temp |0x00000004);


		////config_length=length>>2;
		HAL_PUT_U32((volatile u32 *)(RSA_CMD), 0);
		//temp = HAL_GET_U32((volatile u32 *)(RSA_CMD));
		//HAL_PUT_U32((volatile u32 *)(RSA_CMD), temp & 0xfffffffc);
		for(i=0; i<(64-P_length); i++)
		{
			HAL_PUT_U32((volatile u32 *)(RSA_DAT_IN), 0x00000000);
		}
		for(i=0; i<P_length; i++)
		{
			HAL_PUT_U32((volatile u32 *)(RSA_DAT_IN), P[i]);
		}

		HAL_PUT_U32((volatile u32 *)(RSA_CMD), 1);
		//temp = HAL_GET_U32((volatile u32 *)(RSA_CMD));
		//HAL_PUT_U32((volatile u32 *)(RSA_CMD), (temp & 0xfffffffc)|0x01);
		for(i=0; i<(64-src1_length); i++)
		{
			HAL_PUT_U32((volatile u32 *)(RSA_DAT_IN), 0x00000000);
		}
		for(i=0; i<src1_length; i++)
		{
			HAL_PUT_U32((volatile u32 *)(RSA_DAT_IN), src1[i]);
		}

		HAL_PUT_U32((volatile u32 *)(RSA_CMD), 2);
		//temp = HAL_GET_U32((volatile u32 *)(RSA_CMD));
		//HAL_PUT_U32((volatile u32 *)(RSA_CMD), (temp & 0xfffffffc)|0x02);
		for(i=0; i<(64-src0_length); i++)
		{
			HAL_PUT_U32((volatile u32 *)(RSA_DAT_IN), 0x00000000);
		}
		for(i=0; i<src0_length; i++)
		{
			HAL_PUT_U32((volatile u32 *)(RSA_DAT_IN), src0[i]);
		}

		while ( 0x0000100 & HAL_GET_U32((volatile u32 *)(RSA_CMD)) )
		{
			//printk("sm2 %d calc busy now\n", meth);

			hw_ce_msdelay_customize(1);
			time_out++;
			if(time_out > 6000000)
			{
				printk("sm2 %d calc time out\n", meth);
				return CE_BN_MOD_CALC1_TIMEOUT;
			}
		}

		for(i=0; i<(64-P_length); i++)
		{
			result[0] = HAL_GET_U32((volatile u32 *)(RSA_DAT_OUT));
		}
		for(i=0; i<P_length; i++)
		{
			result[i] = HAL_GET_U32((volatile u32 *)(RSA_DAT_OUT));
		}
	}
        else if (meth == HW_MOD_MOD)
        {
            HW_CE_MISC_CFG_REG misc_cfg_reg;

            if (P_length > 64)
            {
                return CE_BN_MOD_CALC2_LENGTH2_ERROR;
            }

            if (src0_length > 64)
            {
                return CE_BN_MOD_CALC2_LENGTH4_ERROR;
            }

            misc_cfg_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_MISC_CFG));
            misc_cfg_reg.bitc.RSA_CRT_SEL = 1;
            HAL_PUT_U32((volatile u32 *)(CRYPTO_MISC_CFG), misc_cfg_reg.all);

            config_ram_id = 0;
            HAL_PUT_U32((volatile u32 *)(ECC_DAT_CFG), config_ram_id | (64<<8));
            for(i=0; i<(64-P_length); i++)
                HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), 0x00000000);
            for(i=0; i<P_length; i++)
                HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), P[i]);

            config_ram_id = 1;
            if (P_length < src0_length)
            {
                HAL_PUT_U32((volatile u32 *)(ECC_DAT_CFG), config_ram_id | (src0_length<<8));
                for(i=0; i<src0_length; i++)
                    HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), src0[i]);
            }
            else
            {
                HAL_PUT_U32((volatile u32 *)(ECC_DAT_CFG), config_ram_id | (P_length<<8));
                for(i=0; i<(P_length-src0_length); i++)
                    HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), 0x00000000);
                for(i=0; i<src0_length; i++)
                    HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), src0[i]);
            }

            HAL_PUT_U32((volatile u32 *)(ECC_CMD), (1<<28) | (meth<<24) | (5<<16) | (2<<8) | (1));

            while (0x80000000 & HAL_GET_U32((volatile u32 *)(ECC_CMD)))
            {
                hw_ce_msdelay_customize(1);
                time_out++;
                if(time_out > 6000000)
                {
                    printk("sm2 %d calc time out\n", meth);
                    return CE_BN_MOD_CALC2_TIMEOUT;
                }
            }

            for(i=0; i<P_length; i++)
            {
                result[i] = HAL_GET_U32((volatile u32 *)(ECC_DAT_OUT));
            }

            misc_cfg_reg.bitc.RSA_CRT_SEL = 0;
            HAL_PUT_U32((volatile u32 *)(CRYPTO_MISC_CFG), misc_cfg_reg.all);
	}
	else
	{
            HW_CE_MISC_CFG_REG misc_cfg_reg;

            if (src0_length>P_length || src1_length>P_length)
            {
                return CE_BN_MOD_CALC2_LENGTH3_ERROR;
            }

            if (HW_MOD_INV == meth)
            {
                if (P_length > 32)
                {
                    return CE_BN_MOD_CALC2_LENGTH1_ERROR;
                }
            }
            else
            {
                if (P_length > 64)
                {
                    return CE_BN_MOD_CALC2_LENGTH2_ERROR;
                }
            }

            misc_cfg_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_MISC_CFG));
            if (P_length > 16)
            {
                misc_cfg_reg.bitc.RSA_CRT_SEL = 1;
            }
            else
            {
                misc_cfg_reg.bitc.RSA_CRT_SEL = 0;
            }
            HAL_PUT_U32((volatile u32 *)(CRYPTO_MISC_CFG), misc_cfg_reg.all);

            config_ram_id = 0;
            HAL_PUT_U32((volatile u32 *)(ECC_DAT_CFG), config_ram_id | (64<<8));
            for(i=0; i<(64-P_length); i++)
                HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), 0x00000000);
            for(i=0; i<P_length; i++)
                HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), P[i]);

            config_ram_id = 1;
            HAL_PUT_U32((volatile u32 *)(ECC_DAT_CFG), config_ram_id | (P_length<<8));
            for(i=0; i<(P_length-src0_length); i++)
                HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), 0x00000000);
            for(i=0; i<src0_length; i++)
                HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), src0[i]);

            config_ram_id = 2;
            HAL_PUT_U32((volatile u32 *)(ECC_DAT_CFG), config_ram_id | (P_length<<8));
            for(i=0; i<(P_length-src1_length); i++)
                HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), 0x00000000);
            for(i=0; i<src1_length; i++)
                HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), src1[i]);

            HAL_PUT_U32((volatile u32 *)(ECC_CMD), (1<<28) | (meth<<24) | (5<<16) | (2<<8) | (1));

            while (0x80000000 & HAL_GET_U32((volatile u32 *)(ECC_CMD)))
            {
                hw_ce_msdelay_customize(1);
                time_out++;
                if(time_out > 6000000)
                {
                    printk("sm2 %d calc time out\n", meth);
                    return CE_BN_MOD_CALC2_TIMEOUT;
                }
            }

            for(i=0; i<P_length; i++)
            {
                result[i] = HAL_GET_U32((volatile u32 *)(ECC_DAT_OUT));
            }

            misc_cfg_reg.bitc.RSA_CRT_SEL = 0;
            HAL_PUT_U32((volatile u32 *)(CRYPTO_MISC_CFG), misc_cfg_reg.all);
        }

        return CE_SUCCESS;
}

s32 hw_ec_point(HW_EC_METH_E meth, HW_EC_OP_S *op)
{
    u32 i = 0;
    u32 time_out = 0;
    u32 config_ram_id = 0;
    u32 config_length = 0;
    u32 *curve_p = (u32 *)op->curve_p;
    u32 *curve_a = (u32 *)op->curve_a;
    u32 *rsX = (u32 *)op->rsX;
    u32 *rsY = (u32 *)op->rsY;

	HAL_PUT_U32((volatile u32 *)(ECC_ENDIAN), 1);

	config_length = op->length >> 2;
	config_ram_id = 0;
	HAL_PUT_U32((volatile u32 *)(ECC_DAT_CFG), config_ram_id | (16<<8));
	for(i=0; i<(16-config_length); i++)
		HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), 0x00000000);
	for(i=0; i<config_length; i++)
		HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), curve_p[i]);

	config_ram_id = 0x80;
	HAL_PUT_U32((volatile u32 *)(ECC_DAT_CFG), config_ram_id | (16<<8));
	for(i=0; i<(16-config_length); i++)
		HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), 0x00000000);
	for(i=0; i<config_length; i++)
		HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), curve_a[i]);

	if (HW_POINT_MUL == meth)
	{
		u32 *ptX = (u32 *)op->ec_point_mul.ptX;
		u32 *ptY = (u32 *)op->ec_point_mul.ptY;
		u32 *scaler = (u32 *)op->ec_point_mul.scaler;

		config_ram_id = 0x01;
		HAL_PUT_U32((volatile u32 *)(ECC_DAT_CFG), config_ram_id | (config_length<<8));
		for(i=0; i<config_length; i++)
			HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), ptX[i]);

		config_ram_id = 0x41;	//0x21;
		HAL_PUT_U32((volatile u32 *)(ECC_DAT_CFG), config_ram_id | (config_length<<8));
		for(i=0; i<config_length; i++)
			HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), ptY[i]);

		config_ram_id = 0x02;
		HAL_PUT_U32((volatile u32 *)(ECC_DAT_CFG), config_ram_id | (config_length<<8));
		for(i=0; i<config_length; i++)
			HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), scaler[i]);
	}
	else if(HW_POINT_ADD == meth)
	{
		u32 *pt1X = (u32 *)op->ec_point_add.pt1X;
		u32 *pt1Y = (u32 *)op->ec_point_add.pt1Y;
		u32 *pt2X = (u32 *)op->ec_point_add.pt2X;
		u32 *pt2Y = (u32 *)op->ec_point_add.pt2Y;

		config_ram_id = 0x01;
		HAL_PUT_U32((volatile u32 *)(ECC_DAT_CFG), config_ram_id | (config_length<<8));
		for(i=0; i<config_length; i++)
			HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), pt1X[i]);

		config_ram_id = 0x41;	//0x21;
		HAL_PUT_U32((volatile u32 *)(ECC_DAT_CFG), config_ram_id | (config_length<<8));
		for(i=0; i<config_length; i++)
			HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), pt1Y[i]);

		config_ram_id = 0x02;
		HAL_PUT_U32((volatile u32 *)(ECC_DAT_CFG), config_ram_id | (config_length<<8));
		for(i=0; i<config_length; i++)
			HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), pt2X[i]);

		config_ram_id = 0x42;	//0x022;
		HAL_PUT_U32((volatile u32 *)(ECC_DAT_CFG), config_ram_id | (config_length<<8));
		for(i=0; i<config_length; i++)
			HAL_PUT_U32((volatile u32 *)(ECC_DAT_IN), pt2Y[i]);
	}

	HAL_PUT_U32((volatile u32 *)(ECC_CMD), (1<<28) | (meth<<24) | (3<<16) | (2<<8) | (1));

	while (0x80000000 & HAL_GET_U32((volatile u32 *)(ECC_CMD)))
	{
		hw_ce_msdelay_customize(1);
		time_out++;
		if(time_out > 6000000)
		{
			printk("sm2 %d calc time out\n", meth);
			return CE_EC_POINT_CALC_TIMEOUT;
		}
	}

	if(0x40000000 & HAL_GET_U32((volatile u32 *)(ECC_CMD)))
	{
		return CE_EC_POINT_CALC_INFINITY;
	}

	for(i=0; i<config_length; i++)
	{
		rsY[i] = HAL_GET_U32((volatile u32 *)(ECC_DAT_OUT));
	}
	for(i=0; i<config_length; i++)
	{
		rsX[i] = HAL_GET_U32((volatile u32 *)(ECC_DAT_OUT));
	}

	return CE_SUCCESS;
}

s32 hw_ce_reset(u32 mode)
{
    u32 sw_reset_value = HAL_GET_U32((volatile u32 *)(CRYPTO_SW_RST));

    printk("hw reset=%d\n", mode);
    if (mode)
    {
        HAL_PUT_U32((volatile u32 *)(CRYPTO_SW_RST), sw_reset_value & 0xFFFFFFF9);
        hw_ce_msdelay_customize(10000);
        HAL_PUT_U32((volatile u32 *)(CRYPTO_SW_RST), sw_reset_value | 0x00000006);
    }
    else
    {
        HAL_PUT_U32((volatile u32 *)(CRYPTO_SW_RST), sw_reset_value & 0xFFFFFFFE);
        hw_ce_msdelay_customize(10000);
        HAL_PUT_U32((volatile u32 *)(CRYPTO_SW_RST), sw_reset_value | 0x00000001);
    }

    return 0;
}
