/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "drv_ce_if.h"
#include "hw_ce_if.h"

void concerto_flush_dcache_by_vaddr(void *paddr, u32 size);
void concerto_invalidate_dcache_by_vaddr(void *p_addr, u32 size);
void drv_ce_check_private(void *priv);
void drv_ce_channel_lock(mt_session session);
void drv_ce_channel_unlock(mt_session session);
//void drv_ce_kt_lock(void *priv);
//void drv_ce_kt_unlock(void *priv);
MT_BOOL drv_ce_check_keep(mt_session session, u32 same_diff_both);



static mt_s32 hw_ce_ts_config_data_prepare(MT_CE_TS_CTRL_S *app_ctrl, HW_CE_ADES_CTRL_S *hw_ctrl)
{
	switch(app_ctrl->ts_para.work_mode)
	{
	case MT_CE_TS_WORK_MODE_ADES_ECB:
		if (MT_CE_TS_IVE_OFF != app_ctrl->ts_para.ts_ive_mode)
			return CE_TS_ECB_MODE_NOT_NEED_IVE;
		
		hw_ctrl->work_mode = HW_CE_ADES_WORK_MODE_ECB;
		break;
	case MT_CE_TS_WORK_MODE_ADES_CBC:
		hw_ctrl->work_mode = HW_CE_ADES_WORK_MODE_CBC;
		break;
	case MT_CE_TS_WORK_MODE_AES_CTR:
        return CE_NOT_SUPPORT;
	case MT_CE_TS_WORK_MODE_ADES_CBCDVS042:
		hw_ctrl->work_mode = HW_CE_ADES_WORK_MODE_CBCDVS042;
		break;
	case MT_CE_TS_WORK_MODE_ADES_CBCCTS:
		hw_ctrl->work_mode = HW_CE_ADES_WORK_MODE_CBCCTS;
		hw_ctrl->ts_cts_mode = HW_CE_TS_CBCCTS_TSPARSE;
		break;
	case MT_CE_TS_WORK_MODE_ADES_RCBCCTS:
		if (MT_CE_TS_OPERATION_SCRAMBLE == app_ctrl->operation)
			return CE_TS_NOT_SUPPORT_RCBCCTS_ENC; 
		
		hw_ctrl->work_mode = HW_CE_ADES_WORK_MODE_RCBCCTS;
		break;
	case MT_CE_TS_WORK_MODE_ADES_ECBCTS:
		if (MT_CE_TS_OPERATION_SCRAMBLE == app_ctrl->operation)
			return CE_TS_NOT_SUPPORT_ECBCTS_ENC; 
		
		hw_ctrl->work_mode = HW_CE_ADES_WORK_MODE_ECBCTS;
		break;
	default:
		return CE_TS_WORK_MODE_ERROR;
	}
	
	switch(app_ctrl->ts_para.algo_mode)
	{
	case MT_CE_TS_ALGO_MODE_AES128:
		hw_ctrl->algo_sel = HW_CE_ADES_ALGO_SEL_AES;
		if (MT_CE_TS_OPERATION_SCRAMBLE == app_ctrl->operation)
		{
			hw_ctrl->algo_mode = HW_CE_ADES_ALGO_MODE_AES128_ENC;
		}
		else
		{
			hw_ctrl->algo_mode = HW_CE_ADES_ALGO_MODE_AES128_DEC;
		}
		break;
	case MT_CE_TS_ALGO_MODE_DES:
		hw_ctrl->algo_sel = HW_CE_ADES_ALGO_SEL_DES;
		if (MT_CE_TS_OPERATION_SCRAMBLE == app_ctrl->operation)
		{
			hw_ctrl->algo_mode = HW_CE_ADES_ALGO_MODE_DES_XXX_ENC;
		}
		else
		{
			hw_ctrl->algo_mode = HW_CE_ADES_ALGO_MODE_DES_XXX_DEC;
		}
		break;
	case MT_CE_TS_ALGO_MODE_TDES_ABA:
		hw_ctrl->algo_sel = HW_CE_ADES_ALGO_SEL_DES;
		if (MT_CE_TS_OPERATION_SCRAMBLE == app_ctrl->operation)
		{
			hw_ctrl->algo_mode = HW_CE_ADES_ALGO_MODE_TDES_ABA_ENC;
		}
		else
		{
			hw_ctrl->algo_mode = HW_CE_ADES_ALGO_MODE_TDES_ABA_DEC;
		}
		break;
	default:
		return CE_TS_ALGO_MODE_ERROR;
	}

	if(MT_CE_TS_OPERATION_SCRAMBLE == app_ctrl->operation)
	{
		hw_ctrl->ts_enc_dec = HW_CE_TS_ALGO_ENC;
		hw_ctrl->ts_enc_ksel = app_ctrl->ts_para.ts_enc_ksel;
	}
	else
	{
		hw_ctrl->ts_enc_dec = HW_CE_TS_ALGO_DEC;
		hw_ctrl->ts_dec_ind = app_ctrl->ts_para.ts_dec_ind;
	}

	hw_ctrl->ts_on_off = HW_CE_TS_SWITCH_ON;
	hw_ctrl->ts_pkt_len = app_ctrl->ts_para.ts_pkt_len;
	if(MT_CE_TS_IVE_OFF == app_ctrl->ts_para.ts_ive_mode)
	{
		hw_ctrl->ts_ive_cal_en = HW_CE_TS_IVE_CAL_DISABLE;
	}
	else
	{
		hw_ctrl->ts_ive_cal_en = HW_CE_TS_IVE_CAL_ENABLE;
		hw_ctrl->ts_ive_mode = app_ctrl->ts_para.ts_ive_mode;
	}
	hw_ctrl->ts_short_mode = app_ctrl->ts_para.ts_short_mode;
	hw_ctrl->ts_small_mode = app_ctrl->ts_para.ts_small_mode;

	hw_ctrl->even_key_slot = app_ctrl->ts_para.even_key_slot;
	hw_ctrl->odd_key_slot = app_ctrl->ts_para.odd_key_slot;

    //TODO:not used ?
    //hw_ctrl->priv_hdlr = app_ctrl->ts_para.kt_hdlr;

#if 0
	switch(app_ctrl->ts_para.chan_num)
	{
	case MT_CE_CHANNEL_0:
		hw_ctrl->chan_num = HW_CE_CHANNEL_0;
		break;
	case MT_CE_CHANNEL_1:
		hw_ctrl->chan_num = HW_CE_CHANNEL_1;
		break;
	case MT_CE_CHANNEL_2:
		hw_ctrl->chan_num = HW_CE_CHANNEL_2;
		break;
	case MT_CE_CHANNEL_3:
		hw_ctrl->chan_num = HW_CE_CHANNEL_3;
		break;
	case MT_CE_CHANNEL_DEFAULT:
		if (MT_CE_TS_ALGO_MODE_AES128 == app_ctrl->ts_para.algo_mode)
		{
			hw_ctrl->chan_num = HW_CE_CHANNEL_0;
		}
		else if (MT_CE_TS_ALGO_MODE_DES == app_ctrl->ts_para.algo_mode)
		{
			hw_ctrl->chan_num = HW_CE_CHANNEL_1;
		}
		else if (MT_CE_TS_ALGO_MODE_TDES_ABA == app_ctrl->ts_para.algo_mode)
		{
			hw_ctrl->chan_num = HW_CE_CHANNEL_2;
		}
		break;
	default:
		return CE_INVALID_CHANNEL;
	}
#endif

	hw_ctrl->ts_pid0_filt_en = app_ctrl->ts_para.ts_pid0_filt_en;
	hw_ctrl->ts_pid1_filt_en = app_ctrl->ts_para.ts_pid1_filt_en;
	hw_ctrl->ts_pid2_filt_en = app_ctrl->ts_para.ts_pid2_filt_en;
	hw_ctrl->ts_pid3_filt_en = app_ctrl->ts_para.ts_pid3_filt_en;
	hw_ctrl->ts_pid4_filt_en = app_ctrl->ts_para.ts_pid4_filt_en;
	hw_ctrl->ts_pid5_filt_en = app_ctrl->ts_para.ts_pid5_filt_en;
	hw_ctrl->ts_pid6_filt_en = app_ctrl->ts_para.ts_pid6_filt_en;
	hw_ctrl->ts_pid7_filt_en = app_ctrl->ts_para.ts_pid7_filt_en;
	hw_ctrl->ts_pid0_filt_num = app_ctrl->ts_para.ts_pid0_filt_num;
	hw_ctrl->ts_pid1_filt_num = app_ctrl->ts_para.ts_pid1_filt_num;
	hw_ctrl->ts_pid2_filt_num = app_ctrl->ts_para.ts_pid2_filt_num;
	hw_ctrl->ts_pid3_filt_num = app_ctrl->ts_para.ts_pid3_filt_num;
	hw_ctrl->ts_pid4_filt_num = app_ctrl->ts_para.ts_pid4_filt_num;
	hw_ctrl->ts_pid5_filt_num = app_ctrl->ts_para.ts_pid5_filt_num;
	hw_ctrl->ts_pid6_filt_num = app_ctrl->ts_para.ts_pid6_filt_num;
	hw_ctrl->ts_pid7_filt_num = app_ctrl->ts_para.ts_pid7_filt_num;
	hw_ctrl->ts_force_enc_en = app_ctrl->ts_para.ts_force_enc_en;
	return CE_SUCCESS;
}

mt_s32 drv_ce_ts_create(mt_session *p_session, mt_u8 ch, void *p_priv)
{
	drv_ce_check_private(p_priv);
	
	*p_session = (mt_session)kmalloc(sizeof(HW_CE_ADES_CTRL_S), GFP_KERNEL);

	if(0 == *p_session)
	{
		return CE_GET_HANDLE_FAILED;
	}
	
	memset((void *)(*p_session), 0, sizeof(HW_CE_ADES_CTRL_S));
	((HW_CE_ADES_CTRL_S *)(*p_session))->priv = p_priv;
	((HW_CE_ADES_CTRL_S *)(*p_session))->chan_num = ch;
	
	return CE_SUCCESS;
}

mt_s32 drv_ce_ts_destroy(mt_session session)
{
	if(0 == session)
	{
		return CE_INVALID_HANDLE;
	}
	
	drv_ce_check_keep(session, 2);
	
	kfree((void *)session);

	return CE_SUCCESS;
}

mt_s32 drv_ce_ts_config(mt_session session, MT_CE_TS_CTRL_S *p_ctrl)
{
	HW_CE_ADES_CTRL_S *ts_ce = (HW_CE_ADES_CTRL_S *)session;
	mt_s32 ret = CE_SUCCESS;

	if(0 == session)
	{
		return CE_INVALID_HANDLE;
	}
	
	ret = hw_ce_ts_config_data_prepare(p_ctrl, ts_ce);
	
	if(CE_SUCCESS == ret)
	{
		drv_ce_channel_lock(session);
		
		drv_ce_check_keep(session, 0);
		
		drv_ce_channel_unlock(session);
	}

	return ret;
}
#if 0
mt_s32 drv_ce_ts_process(mt_handle_t ce, u8 *p_src_addr, u8 *p_dst_addr, u32 length)
{
	HW_CE_ADES_CTRL_S *ts_ce = (HW_CE_ADES_CTRL_S *)ce;
	
	if(NULL == ce)
	{
		return CE_INVALID_HANDLE;
	}

	drv_ce_channel_lock(ce);
	
	if (drv_ce_check_keep(ce, 1))
	{
		//===================================================
		if (ts_ce->key || ts_ce->iv)
		{
			u32 kt_attribute = 0x001C03ff;
			if (ts_ce->algo_sel == HW_CE_ADES_ALGO_SEL_AES)
			{
				kt_attribute = 0x001C03ff;
			}
			else if (ts_ce->algo_sel == HW_CE_ADES_ALGO_SEL_DES)
			{
				if (HW_CE_ADES_ALGO_MODE_DES_XXX_ENC == ts_ce->algo_mode || HW_CE_ADES_ALGO_MODE_DES_XXX_DEC == ts_ce->algo_mode)
				{
					kt_attribute = 0x000C03ff;
				}
				else
				{
					kt_attribute = 0x001C03ff;
				}
			}
			drv_ce_kt_lock(ts_ce->priv);
			hw_ce_kt_write_attribute(ts_ce->key_slot, kt_attribute);
			hw_ce_kt_write_key(ts_ce->key_slot, ts_ce->key);
			hw_ce_kt_write_iv(ts_ce->key_slot,  ts_ce->iv);
			hw_ce_kt_slot_active(ts_ce->key_slot, 1);
			drv_ce_kt_unlock(ts_ce->priv);
		}
		//===================================================
		hw_ce_ades_config(ce);
	}

	ts_ce->grp_mode = HW_CE_GRP_FIRST_LAST;
	ts_ce->proc_mode = HW_CE_PROC_AUTO;
	concerto_flush_dcache_by_vaddr(p_src_addr, length);
	RET_CODE ret = hw_ce_ades_process(ce, p_src_addr, p_dst_addr, length);
	concerto_invalidate_dcache_by_vaddr(p_dst_addr, length);

	drv_ce_channel_unlock(ce);

	return ret;
}
#else
mt_s32 drv_ce_ts_process(mt_session session, mt_u8 is_phy_addr, mt_u8 *p_src_addr, mt_u8 *p_dst_addr, mt_u32 length)
{
	HW_CE_ADES_CTRL_S *ts_ce = (HW_CE_ADES_CTRL_S *)session;
	mt_s32 ret = CE_SUCCESS;
    mt_u8 *user_in = NULL;
    mt_u8 *user_out = NULL;

	if(0 == session)
	{
		return CE_INVALID_HANDLE;
	}
	
	drv_ce_channel_lock(session);

	if (drv_ce_check_keep(session, 1))
	{
		hw_ce_ades_config(ts_ce);
	}

	ts_ce->grp_mode = HW_CE_GRP_FIRST_LAST;
    if (is_phy_addr) {
        //printk("DMA mode, use addr directly\n");
        ts_ce->proc_mode = HW_CE_PROC_DMA;
        user_in = p_src_addr;
        user_out = p_dst_addr;
    } else {
        //printk("cpu mode, map user pages\n");
        ts_ce->proc_mode = HW_CE_PROC_CPU;
        /* get user data */
        user_in = (mt_u8 *)drv_ce_copy_from_user_data(p_src_addr, length);
        user_out = (mt_u8 *)kmalloc(length, GFP_KERNEL);
    }

    if (user_in && user_out)
        ret = hw_ce_ades_process(ts_ce, user_in, user_out, length);

    if (!is_phy_addr) {
        if (user_in)
            kfree(user_in);

        if (ret == CE_SUCCESS) {
            ret = drv_ce_copy_to_user_data(user_out, p_dst_addr, length);
        }

        if (user_out)
            kfree(user_out);
    }
	
	drv_ce_channel_unlock(session);

	return ret;
}
#endif

mt_s32 drv_ce_ts_get_infor(mt_session session, MT_CE_TS_CTRL_S *p_ctrl)
{
	HW_CE_ADES_CTRL_S *ts_ce = (HW_CE_ADES_CTRL_S *)session;
	
	if(0 == session)
	{
		return CE_INVALID_HANDLE;
	}

	if (HW_CE_ADES_ALGO_SEL_AES == ts_ce->algo_sel)
	{
		if (HW_CE_ADES_ALGO_MODE_AES128_DEC == ts_ce->algo_mode)
		{
			p_ctrl->ts_para.algo_mode = MT_CE_TS_ALGO_MODE_AES128;
			p_ctrl->operation = MT_CE_TS_OPERATION_DESCRAMBLE;
		}
		else if (HW_CE_ADES_ALGO_MODE_AES128_ENC == ts_ce->algo_mode)
		{
			p_ctrl->ts_para.algo_mode = MT_CE_ADES_ALGO_MODE_AES128;
			p_ctrl->operation = MT_CE_TS_OPERATION_SCRAMBLE;
		}
	}
	else if (HW_CE_ADES_ALGO_SEL_DES == ts_ce->algo_sel)
	{
		if (HW_CE_ADES_ALGO_MODE_DES_XXX_DEC == ts_ce->algo_mode)
		{
			p_ctrl->ts_para.algo_mode = MT_CE_TS_ALGO_MODE_DES;
			p_ctrl->operation = MT_CE_TS_OPERATION_DESCRAMBLE;
		}
		else if(HW_CE_ADES_ALGO_MODE_DES_XXX_ENC == ts_ce->algo_mode)
		{
			p_ctrl->ts_para.algo_mode = MT_CE_TS_ALGO_MODE_DES;
			p_ctrl->operation = MT_CE_TS_OPERATION_SCRAMBLE;
		}
		else if(HW_CE_ADES_ALGO_MODE_TDES_ABA_DEC == ts_ce->algo_mode)
		{
			p_ctrl->ts_para.algo_mode = MT_CE_TS_ALGO_MODE_TDES_ABA;
			p_ctrl->operation = MT_CE_TS_OPERATION_DESCRAMBLE;
		}
		else if(HW_CE_ADES_ALGO_MODE_TDES_ABA_ENC == ts_ce->algo_mode)
		{
			p_ctrl->ts_para.algo_mode = MT_CE_TS_ALGO_MODE_TDES_ABA;
			p_ctrl->operation = MT_CE_TS_OPERATION_SCRAMBLE;
		}
	}

	switch(ts_ce->work_mode)
	{
	case HW_CE_ADES_WORK_MODE_ECB:
		if (HW_CE_TS_IVE_CAL_ENABLE == ts_ce->ts_ive_cal_en)
			return CE_TS_ECB_MODE_NOT_NEED_IVE;
		p_ctrl->ts_para.work_mode = MT_CE_TS_WORK_MODE_ADES_ECB;
		break;
	case HW_CE_ADES_WORK_MODE_CBC:
		p_ctrl->ts_para.work_mode = MT_CE_TS_WORK_MODE_ADES_CBC;
		break;
	case HW_CE_ADES_WORK_MODE_CTR:
		if (MT_CE_TS_ALGO_MODE_AES128 != p_ctrl->ts_para.algo_mode)
			return CE_TS_WORK_ALGO_MODE_CONFLICT;
		p_ctrl->ts_para.work_mode = MT_CE_TS_WORK_MODE_AES_CTR;
		break;
	case HW_CE_ADES_WORK_MODE_CBCDVS042:
		p_ctrl->ts_para.work_mode = MT_CE_TS_WORK_MODE_ADES_CBCDVS042;
		break;
	case HW_CE_ADES_WORK_MODE_CBCCTS:
		if (HW_CE_TS_CBCCTS_TSPARSE != ts_ce->ts_cts_mode)
			return -127;
		p_ctrl->ts_para.work_mode = MT_CE_TS_WORK_MODE_ADES_CBCCTS;
		break;
	case HW_CE_ADES_WORK_MODE_RCBCCTS:
		if (MT_CE_TS_OPERATION_SCRAMBLE == p_ctrl->operation)
			return CE_TS_NOT_SUPPORT_RCBCCTS_ENC; 
		p_ctrl->ts_para.work_mode = MT_CE_TS_WORK_MODE_ADES_RCBCCTS;
		break;
	case HW_CE_ADES_WORK_MODE_ECBCTS:
		if (MT_CE_TS_OPERATION_SCRAMBLE == p_ctrl->operation)
			return CE_TS_NOT_SUPPORT_ECBCTS_ENC;
		p_ctrl->ts_para.work_mode = MT_CE_TS_WORK_MODE_ADES_ECBCTS;
		break;
	default:
		return CE_TS_WORK_MODE_ERROR;
	}
	
	switch(ts_ce->chan_num)
	{
	case HW_CE_CHANNEL_0:
		p_ctrl->ts_para.chan_num = MT_CE_CHANNEL_0;
		break;
	case HW_CE_CHANNEL_1:
		p_ctrl->ts_para.chan_num = MT_CE_CHANNEL_1;
		break;
	case HW_CE_CHANNEL_2:
		p_ctrl->ts_para.chan_num = MT_CE_CHANNEL_2;
		break;
	case HW_CE_CHANNEL_3:
		p_ctrl->ts_para.chan_num = MT_CE_CHANNEL_3;
		break;
	default:
		return CE_INVALID_CHANNEL;
	}
	
	p_ctrl->ts_para.even_key_slot = ts_ce->even_key_slot;
	p_ctrl->ts_para.odd_key_slot = ts_ce->odd_key_slot;

    //TODO:not used ?
    //p_ctrl->ts_para.kt_hdlr = ts_ce->priv_hdlr;
	
	return CE_SUCCESS;
}
