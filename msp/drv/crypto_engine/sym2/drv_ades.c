/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/interrupt.h>
#include <linux/wait.h>
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_mach/irq.h"
#include "drv_ce_if.h"
#include "hw_ce_if.h"
#include "hw_ce_register.h"

mt_u32 wq_ades_flag = 0;
DECLARE_WAIT_QUEUE_HEAD(wq_ades);

void concerto_flush_dcache_by_vaddr(void *paddr, mt_u32 size);
void concerto_invalidate_dcache_by_vaddr(void *p_addr, mt_u32 size);
void drv_ce_check_private(void *priv);
void drv_ce_channel_lock(mt_session session);
void drv_ce_channel_unlock(mt_session session);
//void drv_ce_kt_lock(void *priv);
//void drv_ce_kt_unlock(void *priv);
MT_BOOL drv_ce_check_keep(mt_session session, mt_u32 same_diff_both);

#if 0
void dump_ades(mt_u8 *p, mt_u32 len)
{
    unsigned int i = 0;
    printk("=========================\n");
    for (i = 0; i < len; i++) {
        printk("%x ", p[i]);
        if ((i + 1) % 8 == 0) {
            printk("\n");
        }
    }
    printk("=========================\n");
}
#endif

static mt_s32 hw_ce_ades_config_data_prepare(MT_CE_ADES_CTRL_S *app_ctrl, HW_CE_ADES_CTRL_S *hw_ctrl)
{
    hw_ctrl->ts_enc_dec = 0;  //HW_CE_TS_ALGO_ENC;
    hw_ctrl->ts_enc_ksel = 0; //HW_CE_TS_ENC_ODD_KEY;
    hw_ctrl->ts_dec_ind = 0;  //HW_CE_TS_DEC_IND_CLEAR;
    hw_ctrl->ts_pkt_len = 0;  //HW_CE_TS_PKT_LEN_188BYTE;
    hw_ctrl->ts_on_off = HW_CE_TS_SWITCH_OFF;
    hw_ctrl->ts_ive_cal_en = 0; //HW_CE_TS_IVE_CAL_DISABLE;
    hw_ctrl->ts_ive_mode = 0;   //HW_CE_TS_IVE_MDI;
    hw_ctrl->ts_cts_mode = 0;   //HW_CE_TS_CBCCTS_TSPARSE;
    hw_ctrl->ts_short_mode = 0; //HW_CE_TS_SHORT_HEAD;
    hw_ctrl->ts_small_mode = 0; //HW_CE_TS_SMALL_CLEAR;

    switch (app_ctrl->ades_para.work_mode) {
    case MT_CE_ADES_WORK_MODE_ECB:
        hw_ctrl->work_mode = HW_CE_ADES_WORK_MODE_ECB;
        break;
    case MT_CE_ADES_WORK_MODE_CBC:
        hw_ctrl->work_mode = HW_CE_ADES_WORK_MODE_CBC;
        break;
    case MT_CE_AES_WORK_MODE_CTR:
		if (MT_CE_ADES_ALGO_MODE_AES128 != app_ctrl->ades_para.algo_mode)
			return CE_ADES_WORK_ALGO_MODE_CONFLICT;
		
		hw_ctrl->work_mode = HW_CE_ADES_WORK_MODE_CTR;
		break;
    case MT_CE_ADES_WORK_MODE_CBCDVS042:
        hw_ctrl->work_mode = HW_CE_ADES_WORK_MODE_CBCDVS042;
        break;
    case MT_CE_ADES_WORK_MODE_CBCCTS:
        hw_ctrl->work_mode = HW_CE_ADES_WORK_MODE_CBCCTS;
        hw_ctrl->ts_cts_mode = HW_CE_TS_CBCCTS_NORMAL;
        break;
    case MT_CE_ADES_WORK_MODE_CFB1:
    case MT_CE_ADES_WORK_MODE_CFB8:
        return CE_NOT_SUPPORT;
    case MT_CE_ADES_WORK_MODE_CFB:
        hw_ctrl->work_mode = HW_CE_ADES_WORK_MODE_CFB;
        break;
    case MT_CE_ADES_WORK_MODE_OFB1:
    case MT_CE_ADES_WORK_MODE_OFB8:
        return CE_NOT_SUPPORT;
    case MT_CE_ADES_WORK_MODE_OFB:
        hw_ctrl->work_mode = HW_CE_ADES_WORK_MODE_OFB;
        break;
    default:
        return CE_ADES_WORK_MODE_ERROR;
    }

    switch (app_ctrl->ades_para.algo_mode) {
    case MT_CE_ADES_ALGO_MODE_AES128:
        hw_ctrl->algo_sel = HW_CE_ADES_ALGO_SEL_AES;
        if (MT_CE_ADES_OPERATION_ENCRYPT == app_ctrl->operation) {
            hw_ctrl->algo_mode = HW_CE_ADES_ALGO_MODE_AES128_ENC;
        } else {
            hw_ctrl->algo_mode = HW_CE_ADES_ALGO_MODE_AES128_DEC;
        }
        break;
    case MT_CE_ADES_ALGO_MODE_DES:
        hw_ctrl->algo_sel = HW_CE_ADES_ALGO_SEL_DES;
        if (MT_CE_ADES_OPERATION_ENCRYPT == app_ctrl->operation) {
            hw_ctrl->algo_mode = HW_CE_ADES_ALGO_MODE_DES_XXX_ENC;
        } else {
            hw_ctrl->algo_mode = HW_CE_ADES_ALGO_MODE_DES_XXX_DEC;
        }
        break;
    case MT_CE_ADES_ALGO_MODE_TDES_ABA:
        hw_ctrl->algo_sel = HW_CE_ADES_ALGO_SEL_DES;
        if (MT_CE_ADES_OPERATION_ENCRYPT == app_ctrl->operation) {
            hw_ctrl->algo_mode = HW_CE_ADES_ALGO_MODE_TDES_ABA_ENC;
        } else {
            hw_ctrl->algo_mode = HW_CE_ADES_ALGO_MODE_TDES_ABA_DEC;
        }
        break;
    default:
        return CE_ADES_ALGO_MODE_ERROR;
    }

#if 0
	if ( app_ctrl->ades_para.key )
	{
		memcpy(hw_ctrl->key, app_ctrl->ades_para.key, (MT_CE_ADES_ALGO_MODE_DES == app_ctrl->ades_para.algo_mode)?8:16);	//hw_ctrl->key = app_ctrl->ades_para.key;
	}
#endif
    hw_ctrl->even_key_slot = app_ctrl->ades_para.key_slot;

    /* I have got chan_num after create */ 
#if 0
    switch (app_ctrl->ades_para.chan_num) {
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
	if (MT_CE_ADES_ALGO_MODE_AES128 == app_ctrl->ades_para.algo_mode) {
	    hw_ctrl->chan_num = HW_CE_CHANNEL_0;
	} else if (MT_CE_ADES_ALGO_MODE_DES == app_ctrl->ades_para.algo_mode) {
	    hw_ctrl->chan_num = HW_CE_CHANNEL_1;
	} else if (MT_CE_ADES_ALGO_MODE_TDES_ABA == app_ctrl->ades_para.algo_mode) {
	    hw_ctrl->chan_num = HW_CE_CHANNEL_2;
	}
	break;
    default:
	return CE_INVALID_CHANNEL;
    }
#endif

    hw_ctrl->grp_mode = HW_CE_GRP_FIRST_LAST;

    return CE_SUCCESS;
}

mt_s32 drv_ce_ades_create(mt_session *p_session, mt_u8 ch, void *p_priv)
{

    drv_ce_check_private(p_priv);

    *p_session = (mt_session)kmalloc(sizeof(HW_CE_ADES_CTRL_S), GFP_KERNEL);

    if (0 == *p_session) {
	return CE_GET_HANDLE_FAILED;
    }

    memset((void *)(*p_session), 0, sizeof(HW_CE_ADES_CTRL_S));
    ((HW_CE_ADES_CTRL_S *)(*p_session))->priv = p_priv;
    //((HW_CE_ADES_CTRL_S *)(*p_session))->chan_num = CE_INVALID_CHANNEL;
    ((HW_CE_ADES_CTRL_S *)(*p_session))->chan_num = ch;


    return CE_SUCCESS;
}

mt_s32 drv_ce_ades_destroy(mt_session session)
{
    if (0 == session) {
		return CE_INVALID_HANDLE;
    }

	drv_ce_channel_lock(session);
    drv_ce_check_keep(session, 2);
	drv_ce_channel_unlock(session);

    kfree((void *)session);

    return CE_SUCCESS;
}

mt_s32 drv_ce_ades_config(mt_session session, MT_CE_ADES_CTRL_S *p_ctrl)
{
    HW_CE_ADES_CTRL_S *ades_ce = (HW_CE_ADES_CTRL_S *)session;
    mt_s32 ret = CE_SUCCESS;

	if (0 == session) {
		return CE_INVALID_HANDLE;
	}

	ret = hw_ce_ades_config_data_prepare(p_ctrl, ades_ce);
	if (CE_SUCCESS == ret) {
		drv_ce_channel_lock(session);
		drv_ce_check_keep(session, 0);
		drv_ce_channel_unlock(session);
	}

    return ret;
}

mt_s32 drv_ce_ades_process(mt_session session, mt_u8 is_phy_addr, mt_u8 *p_src_addr, mt_u8 *p_dst_addr, mt_u32 length)
{
    HW_CE_ADES_CTRL_S *ades_ce = (HW_CE_ADES_CTRL_S *)session;
    mt_s32 ret = CE_SUCCESS;
    mt_u8 *user_in = NULL;
    mt_u8 *user_out = NULL;


    if (0 == session) {
        return CE_INVALID_HANDLE;
    }

    drv_ce_channel_lock(session);

    if (drv_ce_check_keep(session, 1)) {
        //printk("hw ce_ades_config\n");
        hw_ce_ades_config(ades_ce);
    }
    ades_ce->grp_mode = HW_CE_GRP_FIRST_LAST;

    if (is_phy_addr) {
        ades_ce->proc_mode = HW_CE_PROC_DMA;
        user_in = p_src_addr;
        user_out = p_dst_addr;
    } else {
        //printk("cpu mode, map user pages\n");
        ades_ce->proc_mode = HW_CE_PROC_CPU;
        /* user space data, copy first */
        user_in = (mt_u8 *)drv_ce_copy_from_user_data(p_src_addr, length);
        user_out = (mt_u8 *)kmalloc(length, GFP_KERNEL);

        //dump_ades((mt_u8 *)user_in, length);
    }

    //no need to flush, for input src_addr & dest_addr is physical address already
    //concerto_flush_dcache_by_vaddr(p_src_addr, length);


    //printk("hw ce ades process:src_addr:%x, dest_addr:%x\n", (mt_u32)p_src_addr, (mt_u32)p_dst_addr);
    //printk("hw ce ades process:mappped src_addr:%x, dest_addr:%x\n", (mt_u32)user_in, (mt_u32)user_out);

    if (user_in && user_out)
        ret = hw_ce_ades_process(ades_ce, user_in, user_out, length);
    else
        ret = -1;

    if (!is_phy_addr) {
        //dump_ades((mt_u8 *)user_out, length);
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

mt_s32 drv_ce_ades_grp_process(mt_session session, MT_CE_GRP_MODE_E grp, u8 *p_iv_addr, u8 *p_src_addr, u8 *p_dst_addr, mt_u32 length)
{
	HW_CE_ADES_CTRL_S *ades_ce = (HW_CE_ADES_CTRL_S *)session;
	mt_s32 ret = CE_SUCCESS;

	if (0 == session) {
		return CE_INVALID_HANDLE;
	}

	drv_ce_channel_lock(session);

	if (drv_ce_check_keep(session, 1)) {
		hw_ce_ades_config(ades_ce);
	}

	//=================================================
	switch (grp) {
	case MT_CE_GRP_NONE:
		if (HW_CE_GRP_FIRST_LAST != ades_ce->grp_mode) {
			ret = CE_ADES_GRP_STATE_ERROR;
		}
		break;
	case MT_CE_GRP_FIRST:
		if (HW_CE_GRP_FIRST_LAST == ades_ce->grp_mode) {
			if (length & 0x7F) {
				ret = CE_ADES_GRP_LENGTH_ERROR;
			} else {
				ades_ce->grp_mode = HW_CE_GRP_FIRST;
			}
		} else {
			ret = CE_ADES_GRP_STATE_ERROR;
		}
		break;
	case MT_CE_GRP_LAST:
		if (HW_CE_GRP_FIRST == ades_ce->grp_mode || HW_CE_GRP_MID == ades_ce->grp_mode) {
			ades_ce->grp_mode = HW_CE_GRP_LAST;
		} else {
			ret = CE_ADES_GRP_STATE_ERROR;
		}
		break;
	case MT_CE_GRP_MID:
		if (HW_CE_GRP_FIRST == ades_ce->grp_mode || HW_CE_GRP_MID == ades_ce->grp_mode) {
			if (length & 0x7F) {
				ret = CE_ADES_GRP_LENGTH_ERROR;
			} else {
				ades_ce->grp_mode = HW_CE_GRP_MID;
			}
		} else {
			ret = CE_ADES_GRP_STATE_ERROR;
		}
		break;
	default:
		ret = CE_ADES_GRP_MODE_ERROR;
		break;
	}
	if (CE_SUCCESS != ret) {
		drv_ce_channel_unlock(session);
		return ret;
	}
	//=================================================

	ades_ce->proc_mode = HW_CE_PROC_AUTO;
	concerto_flush_dcache_by_vaddr(p_src_addr, length);

	ret = hw_ce_ades_process(ades_ce, p_src_addr, p_dst_addr, length);
	concerto_invalidate_dcache_by_vaddr(p_dst_addr, length);

	drv_ce_channel_unlock(session);

	return ret;
}

#if 1
mt_s32 drv_ce_ades_process_start(mt_session session, mt_u8 is_phy_addr, mt_u8 *p_src_addr, mt_u8 *p_dst_addr, mt_u32 length)
{
    HW_CE_ADES_CTRL_S *ades_ce = (HW_CE_ADES_CTRL_S *)session;
    mt_s32 ret = CE_SUCCESS;
	mt_u32 mask = 0;

    if (0 == session) {
		return CE_INVALID_HANDLE;
    }

	/* Would be unlocked after stop */
    drv_ce_channel_lock(session);

    if (drv_ce_check_keep(session, 1)) {
		hw_ce_ades_config(ades_ce);
    }

	//TODO:move int policy to ades_config, decided user.
	//Enable interrupt based on ch
	mask = HAL_GET_U32((volatile u32 *)COMINT_MASK);
	mask |= (1 << ades_ce->chan_num);
	HAL_PUT_U32((volatile u32 *)COMINT_MASK, mask);

    ades_ce->grp_mode = HW_CE_GRP_FIRST_LAST;
    ades_ce->proc_mode = HW_CE_PROC_AUTO;
	if (!is_phy_addr) {
		concerto_flush_dcache_by_vaddr(p_src_addr, length);
	}

    ret = hw_ce_ades_process_start(ades_ce, p_src_addr, p_dst_addr, length);

	if (!is_phy_addr) {
		concerto_invalidate_dcache_by_vaddr(p_dst_addr, length);
	}

    return ret;
}

mt_s32 drv_ce_ades_grp_process_start(mt_session session, MT_CE_GRP_MODE_E grp, u8 *p_iv_addr, u8 *p_src_addr, u8 *p_dst_addr, mt_u32 length)
{
    HW_CE_ADES_CTRL_S *ades_ce = (HW_CE_ADES_CTRL_S *)session;
    mt_s32 ret = CE_SUCCESS;

    if (0 == session) {
	return CE_INVALID_HANDLE;
    }

    drv_ce_channel_lock(session);

    if (drv_ce_check_keep(session, 1)) {
#if 0
		//===================================================
		drv_ce_kt_lock(ades_ce->priv);
#if 0
		if (HW_CE_ADES_ALGO_SEL_AES == ades_ce->algo_sel)
		{
			hw_ce_kt_write_attribute(ades_ce->key_slot, 0x001C03ff);
			hw_ce_kt_write_key(ades_ce->key_slot, ades_ce->key, HW_KT_SLOT_16B_SIZE);
		}
		else if (HW_CE_ADES_ALGO_SEL_DES == ades_ce->algo_sel)
		{
			if ((HW_CE_ADES_ALGO_MODE_DES_XXX_ENC == ades_ce->algo_mode) || (HW_CE_ADES_ALGO_MODE_DES_XXX_DEC == ades_ce->algo_mode))
			{
				hw_ce_kt_write_attribute(ades_ce->key_slot, 0x000C03ff);
				hw_ce_kt_write_key(ades_ce->key_slot, ades_ce->key, HW_KT_SLOT_8B_SIZE);
			}
			else
			{
				hw_ce_kt_write_attribute(ades_ce->key_slot, 0x001C03ff);
				hw_ce_kt_write_key(ades_ce->key_slot, ades_ce->key, HW_KT_SLOT_16B_SIZE);
			}
		}
#else
		if (HW_CE_ADES_ALGO_SEL_AES == ades_ce->algo_sel)
		{
			if (HW_CE_ADES_ALGO_MODE_AES128_DEC == ades_ce->algo_mode)
			{
				hw_ce_kt_write_attribute(ades_ce->key_slot, 0x00140081);
				hw_ce_kt_write_key(ades_ce->key_slot, ades_ce->key, HW_KT_SLOT_16B_SIZE);
			}
			else if (HW_CE_ADES_ALGO_MODE_AES128_ENC == ades_ce->algo_mode)
			{
				hw_ce_kt_write_attribute(ades_ce->key_slot, 0x00180081);
				hw_ce_kt_write_key(ades_ce->key_slot, ades_ce->key, HW_KT_SLOT_16B_SIZE);
			}
		}
		else if (HW_CE_ADES_ALGO_SEL_DES == ades_ce->algo_sel)
		{
			if (HW_CE_ADES_ALGO_MODE_DES_XXX_DEC == ades_ce->algo_mode)
			{
				hw_ce_kt_write_attribute(ades_ce->key_slot, 0x00040082);
				hw_ce_kt_write_key(ades_ce->key_slot, ades_ce->key, HW_KT_SLOT_8B_SIZE);
			}
			else if (HW_CE_ADES_ALGO_MODE_DES_XXX_ENC == ades_ce->algo_mode)
			{
				hw_ce_kt_write_attribute(ades_ce->key_slot, 0x00080082);
				hw_ce_kt_write_key(ades_ce->key_slot, ades_ce->key, HW_KT_SLOT_8B_SIZE);
			}
			else if (HW_CE_ADES_ALGO_MODE_TDES_ABA_DEC == ades_ce->algo_mode)
			{
				hw_ce_kt_write_attribute(ades_ce->key_slot, 0x02140084);
				hw_ce_kt_write_key(ades_ce->key_slot, ades_ce->key, HW_KT_SLOT_16B_SIZE);
			}
			else if (HW_CE_ADES_ALGO_MODE_TDES_ABA_ENC == ades_ce->algo_mode)
			{
				hw_ce_kt_write_attribute(ades_ce->key_slot, 0x02180084);
				hw_ce_kt_write_key(ades_ce->key_slot, ades_ce->key, HW_KT_SLOT_16B_SIZE);
			}
		}
#endif
		hw_ce_kt_slot_active(ades_ce->key_slot, 1);
		
		drv_ce_kt_unlock(ades_ce->priv);
		//===================================================
#endif
	hw_ce_ades_config(ades_ce);
    }
#if 0
	if (p_iv_addr)
	{
		memcpy(ades_ce->iv, p_iv_addr, (ades_ce->algo_sel == HW_CE_ADES_ALGO_SEL_AES)?16:8);
		
		drv_ce_kt_lock(ades_ce->priv);
		
		if (HW_CE_ADES_ALGO_SEL_AES == ades_ce->algo_sel)
		{
			hw_ce_kt_write_iv(ades_ce->key_slot,  ades_ce->iv, HW_KT_SLOT_16B_SIZE);
		}
		else if (HW_CE_ADES_ALGO_SEL_DES == ades_ce->algo_sel)
		{
			hw_ce_kt_write_iv(ades_ce->key_slot,  ades_ce->iv, HW_KT_SLOT_8B_SIZE);
		}
		hw_ce_kt_slot_active(ades_ce->key_slot, 1);
		
		drv_ce_kt_unlock(ades_ce->priv);
	}
#endif
    //=================================================
    switch (grp) {
    case MT_CE_GRP_NONE:
	if (HW_CE_GRP_FIRST_LAST != ades_ce->grp_mode) {
	    ret = CE_ADES_GRP_STATE_ERROR;
	}
	break;
    case MT_CE_GRP_FIRST:
	if (HW_CE_GRP_FIRST_LAST == ades_ce->grp_mode) {
	    if (length & 0x7F) {
		ret = CE_ADES_GRP_LENGTH_ERROR;
	    } else {
		ades_ce->grp_mode = HW_CE_GRP_FIRST;
	    }
	} else {
	    ret = CE_ADES_GRP_STATE_ERROR;
	}
	break;
    case MT_CE_GRP_LAST:
	if (HW_CE_GRP_FIRST == ades_ce->grp_mode || HW_CE_GRP_MID == ades_ce->grp_mode) {
	    ades_ce->grp_mode = HW_CE_GRP_LAST;
	} else {
	    ret = CE_ADES_GRP_STATE_ERROR;
	}
	break;
    case MT_CE_GRP_MID:
	if (HW_CE_GRP_FIRST == ades_ce->grp_mode || HW_CE_GRP_MID == ades_ce->grp_mode) {
	    if (length & 0x7F) {
		ret = CE_ADES_GRP_LENGTH_ERROR;
	    } else {
		ades_ce->grp_mode = HW_CE_GRP_MID;
	    }
	} else {
	    ret = CE_ADES_GRP_STATE_ERROR;
	}
	break;
    default:
	ret = CE_ADES_GRP_MODE_ERROR;
	break;
    }
    if (CE_SUCCESS != ret) {
	drv_ce_channel_unlock(session);
	return ret;
    }
    //=================================================

    ades_ce->proc_mode = HW_CE_PROC_AUTO;
    concerto_flush_dcache_by_vaddr(p_src_addr, length);

    ret = hw_ce_ades_process_start(ades_ce, p_src_addr, p_dst_addr, length);
    concerto_invalidate_dcache_by_vaddr(p_dst_addr, length);

    return ret;
}

mt_s32 drv_ce_ades_process_polling(mt_session session, mt_u32 time_out)
{
	HW_CE_ADES_CTRL_S *ades_ce = (HW_CE_ADES_CTRL_S *)session;
	mt_s32 ret = CE_SUCCESS;

	if (0 == session) {
		return CE_INVALID_HANDLE;
	}

	ret = hw_ce_ades_process_polling(ades_ce, 0);

	return ret;
}

mt_s32 drv_ce_ades_process_stop(mt_session session)
{
	HW_CE_ADES_CTRL_S *ades_ce = (HW_CE_ADES_CTRL_S *)session;
	mt_s32 ret = CE_SUCCESS;

	if (0 == session) {
		return CE_INVALID_HANDLE;
	}

	ret = hw_ce_ades_process_stop(ades_ce);

	/* This is locked when process_start, should be unlocked here */
	drv_ce_channel_unlock(session);

	return ret;
}


mt_s32 drv_ce_ades_process_wait(mt_session session, mt_u32 timeout)
{
	mt_s32 ret = 0;
    HW_CE_ADES_CTRL_S *ades_ce = (HW_CE_ADES_CTRL_S *)session;
	mt_u8 ch = 0;
	mt_u32 mask = 0;

	/* Get channel */
    ch = ades_ce->chan_num;

	/* Waiting signal */
	while (timeout--) {
		/* Only bit0,1,2,3 are available */
		wait_event_interruptible(wq_ades, ((wq_ades_flag & 0xF) != 0));
		/* Check if this event is sent to me */
		if ((wq_ades_flag >> ch) & 0x1) {
			wq_ades_flag &= ~(1 << ch);
			break;
		}
		/* keep on waiting */
	}

	//TODO:move int policy to ades_config, decided user.
	//Disable interrupt based on ch
	mask = HAL_GET_U32((volatile u32 *)COMINT_MASK);
	mask &= ~(1 << ch);
	HAL_PUT_U32((volatile u32 *)COMINT_MASK, mask);


	return ret;
}
#endif

mt_s32 drv_ce_ades_get_infor(mt_session session, MT_CE_ADES_CTRL_S *p_ctrl)
{
    HW_CE_ADES_CTRL_S *ades_ce = (HW_CE_ADES_CTRL_S *)session;

    if (0 == session) {
	return CE_INVALID_HANDLE;
    }

    if (HW_CE_ADES_ALGO_SEL_AES == ades_ce->algo_sel) {
	if (HW_CE_ADES_ALGO_MODE_AES128_DEC == ades_ce->algo_mode) {
	    p_ctrl->ades_para.algo_mode = MT_CE_ADES_ALGO_MODE_AES128;
	    p_ctrl->operation = MT_CE_ADES_OPERATION_DECRYPT;
	} else if (HW_CE_ADES_ALGO_MODE_AES128_ENC == ades_ce->algo_mode) {
	    p_ctrl->ades_para.algo_mode = MT_CE_ADES_ALGO_MODE_AES128;
	    p_ctrl->operation = MT_CE_ADES_OPERATION_ENCRYPT;
	}
    } else if (HW_CE_ADES_ALGO_SEL_DES == ades_ce->algo_sel) {
	if (HW_CE_ADES_ALGO_MODE_DES_XXX_DEC == ades_ce->algo_mode) {
	    p_ctrl->ades_para.algo_mode = MT_CE_ADES_ALGO_MODE_DES;
	    p_ctrl->operation = MT_CE_ADES_OPERATION_DECRYPT;
	} else if (HW_CE_ADES_ALGO_MODE_DES_XXX_ENC == ades_ce->algo_mode) {
	    p_ctrl->ades_para.algo_mode = MT_CE_ADES_ALGO_MODE_DES;
	    p_ctrl->operation = MT_CE_ADES_OPERATION_ENCRYPT;
	} else if (HW_CE_ADES_ALGO_MODE_TDES_ABA_DEC == ades_ce->algo_mode) {
	    p_ctrl->ades_para.algo_mode = MT_CE_ADES_ALGO_MODE_TDES_ABA;
	    p_ctrl->operation = MT_CE_ADES_OPERATION_DECRYPT;
	} else if (HW_CE_ADES_ALGO_MODE_TDES_ABA_ENC == ades_ce->algo_mode) {
	    p_ctrl->ades_para.algo_mode = MT_CE_ADES_ALGO_MODE_TDES_ABA;
	    p_ctrl->operation = MT_CE_ADES_OPERATION_ENCRYPT;
	}
    }

    switch (ades_ce->work_mode) {
    case HW_CE_ADES_WORK_MODE_ECB:
	p_ctrl->ades_para.work_mode = MT_CE_ADES_WORK_MODE_ECB;
	break;
    case HW_CE_ADES_WORK_MODE_CBC:
	p_ctrl->ades_para.work_mode = MT_CE_ADES_WORK_MODE_CBC;
	break;
    case HW_CE_ADES_WORK_MODE_CTR:
	if (MT_CE_ADES_ALGO_MODE_AES128 != p_ctrl->ades_para.algo_mode)
	    return CE_ADES_WORK_ALGO_MODE_CONFLICT;

	p_ctrl->ades_para.work_mode = MT_CE_AES_WORK_MODE_CTR;
    case HW_CE_ADES_WORK_MODE_CBCDVS042:
	p_ctrl->ades_para.work_mode = MT_CE_ADES_WORK_MODE_CBCDVS042;
	break;
    case HW_CE_ADES_WORK_MODE_CBCCTS:
	p_ctrl->ades_para.work_mode = MT_CE_ADES_WORK_MODE_CBCCTS;
	break;
    case HW_CE_ADES_WORK_MODE_CFB1:
	p_ctrl->ades_para.work_mode = MT_CE_ADES_WORK_MODE_CFB1;
	break;
    case HW_CE_ADES_WORK_MODE_CFB8:
	p_ctrl->ades_para.work_mode = MT_CE_ADES_WORK_MODE_CFB8;
	break;
    case HW_CE_ADES_WORK_MODE_CFB:
	p_ctrl->ades_para.work_mode = MT_CE_ADES_WORK_MODE_CFB;
	break;
    case HW_CE_ADES_WORK_MODE_OFB1:
	p_ctrl->ades_para.work_mode = MT_CE_ADES_WORK_MODE_OFB1;
	break;
    case HW_CE_ADES_WORK_MODE_OFB8:
	p_ctrl->ades_para.work_mode = MT_CE_ADES_WORK_MODE_OFB8;
	break;
    case HW_CE_ADES_WORK_MODE_OFB:
	p_ctrl->ades_para.work_mode = MT_CE_ADES_WORK_MODE_OFB;
	break;
    default:
	return CE_ADES_WORK_MODE_ERROR;
    }

    switch (ades_ce->chan_num) {
    case HW_CE_CHANNEL_0:
	p_ctrl->ades_para.chan_num = MT_CE_CHANNEL_0;
	break;
    case HW_CE_CHANNEL_1:
	p_ctrl->ades_para.chan_num = MT_CE_CHANNEL_1;
	break;
    case HW_CE_CHANNEL_2:
	p_ctrl->ades_para.chan_num = MT_CE_CHANNEL_2;
	break;
    case HW_CE_CHANNEL_3:
	p_ctrl->ades_para.chan_num = MT_CE_CHANNEL_3;
	break;
    default:
	return CE_INVALID_CHANNEL;
    }

    p_ctrl->ades_para.key_slot = ades_ce->even_key_slot;

    return CE_SUCCESS;
}

static irqreturn_t ce_isr(int irq, void *dev)
{
	mt_u32 sta = 0;

	//get int state and clear
	sta = HAL_GET_U32((volatile u32 *)COMINT_STA);
	//ch0,1,2,3
	wq_ades_flag = sta & 0xF;

	//signal the peer
	wake_up_interruptible(&wq_ades);

	return IRQ_HANDLED;
}

void drv_ce_ades_init(void)
{
	mt_u32 sta = 0;
	int ret = 0;

	/* Interrupt disable, clear state */
	HAL_PUT_U32((volatile u32 *)COMINT_MASK, 0);
	sta = HAL_GET_U32((volatile u32 *)COMINT_STA);

	ret = request_irq(IRQ_SECURE_ID, (irq_handler_t)ce_isr, IRQF_TRIGGER_RISING, "ce_int", NULL);
	if (ret != 0) {
		printk(KERN_ERR "CryptoEngine irq request failed,ret:%x\n", ret);
	}
}

void drv_ce_ades_deinit(void)
{
	free_irq(IRQ_SECURE_ID, NULL);
}
