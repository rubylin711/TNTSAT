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



#define HW_CE_ALGO_SEL_SHA		2

s32 hw_ce_sha_init(HW_CE_SHA_CTRL_S *hw_ctrl)
{
	HW_CE_ALGO_MODE_REG mode_reg;
	mode_reg.all = HAL_GET_U32((volatile u32 *)(ALGORITHMn_MODE(hw_ctrl->chan_num)));

	mode_reg.bitc.TSPARSE_DEC = 0;
	mode_reg.bitc.TSPARSE_KEY_SEL = 0;
	mode_reg.bitc.TSPARSE_IND = 0;
	mode_reg.bitc.TSPARSE_LEN = 0;
	mode_reg.bitc.TSPARSE_EN = 0;
	mode_reg.bitc.TSPARSE_IVE_CAL_EN = 0;
	mode_reg.bitc.TSPARSE_IVE_MODE = 0;
	mode_reg.bitc.TSPARSE_CTS_MODE = 0;
	mode_reg.bitc.TSPARSE_SHORT_MODE = 0;
	mode_reg.bitc.TSPARSE_SMALL_MODE = 0;
	mode_reg.bitc.ADES_MODE = 0;

	mode_reg.bitc.ALGORITHM_MODE = hw_ctrl->algo_mode;
	mode_reg.bitc.ALGORITHM_SEL = HW_CE_ALGO_SEL_SHA;
	HAL_PUT_U32((volatile u32 *)(ALGORITHMn_MODE(hw_ctrl->chan_num)), mode_reg.all);

	if (HW_CE_HASH_ALGO_MODE_HMAC_SHA256 == hw_ctrl->algo_mode)
	{
		HAL_PUT_U32((volatile u32 *)(ALGORITHM_EVEN_KEY_ADDR(hw_ctrl->chan_num)), hw_ctrl->key_slot);
		HAL_PUT_U32((volatile u32 *)(ALGORITHM_ODD_KEY_ADDR(hw_ctrl->chan_num)), hw_ctrl->key_slot);
	}

	return CE_SUCCESS;
}

s32 hw_ce_sha_update(HW_CE_SHA_CTRL_S *hw_ctrl, u8 *p_msg, u32 length)
{
	u32 i, j, timeout, sum_length=0, wordlength=(length+3)>>2;
	u32 *input_data = (u32 *)p_msg;
	u32 input_addr = (u32)p_msg;
	HW_CE_CHANNEL_CFG_REG chan_cfg_reg;
	HW_CE_SHA_GRP_REG grp_mod_reg;

	chan_cfg_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)));
	grp_mod_reg.all = HAL_GET_U32((volatile u32 *)(SHA_GRP_CTRL));

	if (length == 0)
		return CE_SHA_MSG_EMPTYLOAD_ERROR;

	if ((length&0x7F) && (HW_CE_HASH_GRP_FIRST==hw_ctrl->grp_mode || HW_CE_HASH_GRP_MID==hw_ctrl->grp_mode))
		return CE_SHA_MSG_GRPLENGTH_ERROR;

	HAL_PUT_U32((volatile u32 *)(ALGORITHMn_LEN(hw_ctrl->chan_num)), length);
	grp_mod_reg.bitc.GRP_MODE = hw_ctrl->grp_mode;
	HAL_PUT_U32((volatile u32 *)(SHA_GRP_CTRL), grp_mod_reg.all);

	if( HW_CE_HASH_GRP_NONE != hw_ctrl->grp_mode )
	{
		hw_ctrl->grp_ctx.total_length += length;

		if(HW_CE_HASH_GRP_LAST == hw_ctrl->grp_mode)
		{
			HAL_PUT_U32((volatile u32 *)(SHA_LEN_SUM), hw_ctrl->grp_ctx.total_length);
		}

		if(HW_CE_HASH_GRP_FIRST != hw_ctrl->grp_mode)
		{
			for(i=0; i<8; i++)
			{
				HAL_PUT_U32((volatile u32 *)(SHA_REG(i)), hw_ctrl->grp_ctx.state[i]);
			}
		}
	}

	if ( (HW_CE_PROC_CPU == hw_ctrl->proc_mode) || ((HW_CE_PROC_AUTO == hw_ctrl->proc_mode) && (CHK_IF_CPU_MODE_SHA)) )
	{
        //printk("[%s:%d] sha cpu mode\n", __FUNCTION__, __LINE__);
		/******************* sha cpu mode ***************************/
		if((input_addr & 0x00000003))
		{
		//	printk("sha at cpu mode: message address is not 4bytes multiple\n");
			return CE_SHA_CPU_ADDR_ERROR;
		}

		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_BUS_MODE), 0x0f0);
		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_LEN(hw_ctrl->chan_num)), length);

		chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
		chan_cfg_reg.bitc.DMA_CHn_MODE_CFG = 1;
		chan_cfg_reg.bitc.DMA_CHn_ENABLE = 1;
		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);

		for(i=0; i<wordlength; i++)
		{
			if(0 == (i&0x1F))
			{
				if (length > HW_CE_FIFO_DEPTH)
				{
					sum_length += HW_CE_FIFO_DEPTH;
					length -= HW_CE_FIFO_DEPTH;
				}
				else
				{
					sum_length += length;
					length = 0;
				}
			}

			timeout = 0;
			while (HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_IBUF(hw_ctrl->chan_num))) < (sum_length - i*4))
			{
				hw_ce_msdelay_customize(1);
				timeout++;
                if (timeout > 5000)
				{
					printk("====[%s:%d]CE_SHA_CPU_WRITE_TIMEOUT====\n", __FUNCTION__, __LINE__);
                    chan_cfg_reg.bitc.DMA_CHn_CANCEL = 0;
					chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
					HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);	//close operation
					return CE_SHA_CPU_WRITE_TIMEOUT;
				}
			}

			if (input_addr & 0x3)
			{
				j=i*4;
				HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_WSLV(hw_ctrl->chan_num)), (p_msg[j+3]<<24) + (p_msg[j+2]<<16) + (p_msg[j+1]<<8) + (p_msg[j]));
			}
			else
			{
				HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_WSLV(hw_ctrl->chan_num)), input_data[i]);
			}
		}
	}
	else
	{
		/****************************** sha dma mode *********************************************************/
        //printk("[%s:%d] sha dma mode\n", __FUNCTION__, __LINE__);
		if(input_addr & 0x00000007)
		{
			printk("sha at dma mode: message address is not 8bytes multiple\n");
			return CE_SHA_DMA_ADDR_ERROR;
		}

		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_SRC(hw_ctrl->chan_num)), (input_addr & 0x1FFFFFFF)>>3);
		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_LEN(hw_ctrl->chan_num)), length);

		chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
		chan_cfg_reg.bitc.DMA_CHn_MODE_CFG = 0;
		chan_cfg_reg.bitc.DMA_CHn_ENABLE = 1;
		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);
	}

	//=============================================================
	timeout = 0;
    //hw_ce_msdelay_customize(10);
    mdelay(5);
	while ((HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_STATE(hw_ctrl->chan_num))) & 0x00000001) 
            || (HAL_GET_U32((volatile u32 *)(ALGORITHM_STATUS)) & 0x00000010)) {
        //hw_ce_msdelay_customize(1);
        msleep(10);
		timeout++;
        if (timeout > 20000)
		{
            printk("====[%s:%d]CE_SHA_CPU_WRITE_TIMEOUT====\n", __FUNCTION__, __LINE__);
			chan_cfg_reg.bitc.DMA_CHn_CANCEL = 0;
			chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
			HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);		//close operation
			return CE_SHA_CALC_TIMEOUT;
		}
	}
	for (i = 0; i < 8; i++) {
		hw_ctrl->grp_ctx.state[i] = HAL_GET_U32((volatile u32 *)(SHA_REG(i)));
	}

	chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);				//close operation
	return CE_SUCCESS;
}

s32 hw_ce_sha_final(HW_CE_SHA_CTRL_S *hw_ctrl, u8 *p_dgst)
{
	u32 i=0, length=0, wordlength=0;
	u32 output_data[8];
	u8 *p_output = (u8 *)output_data;
	HW_CE_CHANNEL_CFG_REG chan_cfg_reg;
	HW_CE_ALGO_MODE_REG mode_reg;

	chan_cfg_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)));
	mode_reg.all = HAL_GET_U32((volatile u32 *)(ALGORITHMn_MODE(hw_ctrl->chan_num)));

	switch(mode_reg.bitc.ALGORITHM_MODE | (mode_reg.bitc.ALGORITHM_SEL<<3))
	{
	case HW_CE_HASH_ALGO_MODE_SHA1 | HW_CE_ALGO_SEL_SHA<<3:
		length = 20;
		wordlength = 5;
		break;
//	case HW_CE_HASH_ALGO_MODE_SHA224 | HW_CE_ALGO_SEL_SHA<<3:
//		length = 28;
//		wordlength = 7;
//		break;
	case HW_CE_HASH_ALGO_MODE_SHA256 | HW_CE_ALGO_SEL_SHA<<3:
		length = 32;
		wordlength = 8;
		break;
	case HW_CE_HASH_ALGO_MODE_HMAC_SHA256 | HW_CE_ALGO_SEL_SHA<<3:
		length = 32;
		wordlength = 8;
		break;
	default:
		chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);		//close operation
		return CE_SHA_INIT_FAILED;
	}

	for(i=0; i<wordlength; i++)
	{
		output_data[i] = HAL_GET_U32((volatile u32 *)( SHA_REG(i) ));
	}

	for(i=0; i<length; i++)
	{
		p_dgst[i] = p_output[i];
	}

	return CE_SUCCESS;
}

#if 1
s32 hw_ce_sha_update_start(HW_CE_SHA_CTRL_S *hw_ctrl, u8 *p_msg, u32 length)
{
	u32 i, j, timeout, sum_length=0, wordlength=(length+3)>>2;
	u32 *input_data = (u32 *)p_msg;
	u32 input_addr = (u32)p_msg;
	HW_CE_CHANNEL_CFG_REG chan_cfg_reg;
	HW_CE_SHA_GRP_REG grp_mod_reg;

	chan_cfg_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)));
	grp_mod_reg.all = HAL_GET_U32((volatile u32 *)(SHA_GRP_CTRL));

	if (length == 0)
		return CE_SHA_MSG_EMPTYLOAD_ERROR;

//	if (length > HW_CE_HASH_MSG_LENGTH_MAX)
//		return CE_SHA_MSG_OVERLOAD_ERROR;

	if ((length&0x7F) && (HW_CE_HASH_GRP_FIRST==hw_ctrl->grp_mode || HW_CE_HASH_GRP_MID==hw_ctrl->grp_mode))
		return CE_SHA_MSG_GRPLENGTH_ERROR;

	HAL_PUT_U32((volatile u32 *)(ALGORITHMn_LEN(hw_ctrl->chan_num)), length);
	grp_mod_reg.bitc.GRP_MODE = hw_ctrl->grp_mode;
	HAL_PUT_U32((volatile u32 *)(SHA_GRP_CTRL), grp_mod_reg.all);

	if( HW_CE_HASH_GRP_NONE != hw_ctrl->grp_mode )
	{
		hw_ctrl->grp_ctx.total_length += length;

		if(HW_CE_HASH_GRP_LAST == hw_ctrl->grp_mode)
		{
			HAL_PUT_U32((volatile u32 *)(SHA_LEN_SUM), hw_ctrl->grp_ctx.total_length);
		}

		if(HW_CE_HASH_GRP_FIRST != hw_ctrl->grp_mode)
		{
			for(i=0; i<8; i++)
			{
				HAL_PUT_U32((volatile u32 *)(SHA_REG(i)), hw_ctrl->grp_ctx.state[i]);
			}
		}
	}

	if ( (HW_CE_PROC_CPU == hw_ctrl->proc_mode) || ((HW_CE_PROC_AUTO == hw_ctrl->proc_mode) && (CHK_IF_CPU_MODE_SHA)) )
	{
		//printk("sha cpu mode\n");
		if((input_addr & 0x00000003))
		{
			printk("sha at cpu mode: message address is not 4bytes multiple\n");
			return CE_SHA_CPU_ADDR_ERROR;
		}

		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_BUS_MODE), 0x0f0);
		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_LEN(hw_ctrl->chan_num)), length);

		chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
		chan_cfg_reg.bitc.DMA_CHn_MODE_CFG = 1;
		chan_cfg_reg.bitc.DMA_CHn_ENABLE = 1;
		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);

		for(i=0; i<wordlength; i++)
		{
			if(0 == (i&0x1F))
			{
				if (length > HW_CE_FIFO_DEPTH)
				{
					sum_length += HW_CE_FIFO_DEPTH;
					length -= HW_CE_FIFO_DEPTH;
				}
				else
				{
					sum_length += length;
					length = 0;
				}
			}

			timeout = 0;
			while (HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_IBUF(hw_ctrl->chan_num))) < (sum_length - i*4))
			{
				hw_ce_msdelay_customize(1);
				timeout++;
				if (timeout > 5000)
				{
					printk("====CE_SHA_CPU_WRITE_TIMEOUT====\n");
					chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
					HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);	//close operation
					return CE_SHA_CPU_WRITE_TIMEOUT;
				}
			}

			if (input_addr & 0x3)
			{
				j=i*4;
				HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_WSLV(hw_ctrl->chan_num)), (p_msg[j+3]<<24) + (p_msg[j+2]<<16) + (p_msg[j+1]<<8) + (p_msg[j]));
			}
			else
			{
				HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_WSLV(hw_ctrl->chan_num)), input_data[i]);
			}
		}
	}
	else
	{
		//printk("sha dma mode\n");
		if(input_addr & 0x00000007)
		{
			printk("sha at dma mode: message address is not 8bytes multiple\n");
			return CE_SHA_DMA_ADDR_ERROR;
		}

		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_SRC(hw_ctrl->chan_num)), (input_addr & 0x1FFFFFFF)>>3);
		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_LEN(hw_ctrl->chan_num)), length);

		chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
		chan_cfg_reg.bitc.DMA_CHn_MODE_CFG = 0;
		chan_cfg_reg.bitc.DMA_CHn_ENABLE = 1;
		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);
	}

	//=============================================================
if (HW_CE_HASH_GRP_MID!=hw_ctrl->grp_mode && HW_CE_HASH_GRP_NONE!=hw_ctrl->grp_mode)
{
	timeout = 0;
	hw_ce_msdelay_customize(10);
	while((HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_STATE(hw_ctrl->chan_num))) & 0x00000001) || (HAL_GET_U32((volatile u32 *)(ALGORITHM_STATUS)) & 0x00000010))
	{
		hw_ce_msdelay_customize(1);
		timeout++;
		if (timeout > 20000)
		{
			printk("====CE_SHA_CALC_TIMEOUT====\n");

			chan_cfg_reg.bitc.DMA_CHn_CANCEL = 0;
			chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
			HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);		//close operation
			return CE_SHA_CALC_TIMEOUT;
		}
	}

	for(i=0; i<8; i++)
	{
		hw_ctrl->grp_ctx.state[i] = HAL_GET_U32((volatile u32 *)(SHA_REG(i)));
	}

	chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);				//close operation
}
	return CE_SUCCESS;
}

s32 hw_ce_sha_update_polling(HW_CE_SHA_CTRL_S *hw_ctrl, u32 time_out)
{
	u32 i, timeout = 0;
	HW_CE_CHANNEL_CFG_REG chan_cfg_reg;
	chan_cfg_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)));

	if (HW_CE_HASH_GRP_MID == hw_ctrl->grp_mode || HW_CE_HASH_GRP_NONE == hw_ctrl->grp_mode)
	{
		hw_ce_msdelay_customize(10);
		while((HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_STATE(hw_ctrl->chan_num))) & 0x00000001) || (HAL_GET_U32((volatile u32 *)(ALGORITHM_STATUS)) & 0x00000010))
		{
			hw_ce_msdelay_customize(1);
			timeout++;
			if (timeout > time_out)
			{
				//printk("====ch%d CE SHA STILL BUSY NOW====\n", hw_ctrl->chan_num);
				return CE_SHA_CALC_TIMEOUT;
			}
		}

		for(i=0; i<8; i++)
		{
			hw_ctrl->grp_ctx.state[i] = HAL_GET_U32((volatile u32 *)(SHA_REG(i)));
		}

		chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);				//close operation

		return CE_SUCCESS;
	}
	else
	{
		return CE_SHA_POLLING_IN_INVALID_GRP;
	}
}

s32 hw_ce_sha_update_stop(HW_CE_SHA_CTRL_S *hw_ctrl)
{
	HW_CE_CHANNEL_CFG_REG chan_cfg_reg;
	chan_cfg_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)));

	chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);				//close operation

	return CE_SUCCESS;
}

#endif

