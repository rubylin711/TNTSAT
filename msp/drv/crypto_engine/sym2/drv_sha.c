/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <asm/io.h>
//#include "drv_kt_if.h"
#include "drv_ce_if.h"
#include "hw_ce_if.h"
/*#include "mt_mpi_kt.h"*/

void concerto_flush_dcache_by_vaddr(void *paddr, u32 size);
void concerto_invalidate_dcache_by_vaddr(void *p_addr, u32 size);
void drv_ce_check_private(void *priv);
void drv_ce_channel_lock(mt_session session);
void drv_ce_channel_unlock(mt_session session);
//void drv_ce_kt_lock(void *priv);
//void drv_ce_kt_unlock(void *priv);
MT_BOOL drv_ce_check_keep(mt_session session, u32 same_diff_both);


static s32 hw_ce_sha_config_data_prepare(MT_CE_SHA_CTRL_S *app_ctrl, HW_CE_SHA_CTRL_S *hw_ctrl)
{
	switch(app_ctrl->sha_para.algo_mode)
	{
	case MT_CE_HASH_ALGO_MODE_SHA1:
		hw_ctrl->algo_mode = HW_CE_HASH_ALGO_MODE_SHA1;
		break;
//	case MT_CE_HASH_ALGO_MODE_SHA224:
//		hw_ctrl->algo_mode = HW_CE_HASH_ALGO_MODE_SHA224;
//		break;
	case MT_CE_HASH_ALGO_MODE_SHA256:
		hw_ctrl->algo_mode = HW_CE_HASH_ALGO_MODE_SHA256;
		hw_ctrl->grp_mode = HW_CE_HASH_GRP_NONE;
		hw_ctrl->proc_mode = HW_CE_PROC_AUTO;
		memset(&(hw_ctrl->grp_ctx), 0, sizeof(HW_CE_SHA_GRP_CTX_S));
		memset(&(hw_ctrl->mgr_ctx), 0, sizeof(SW_CE_SHA_MGR_CTX_S));
		break;
	case MT_CE_HASH_ALGO_MODE_HMAC_SHA256:
		hw_ctrl->algo_mode = HW_CE_HASH_ALGO_MODE_HMAC_SHA256;
		hw_ctrl->grp_mode = HW_CE_HASH_GRP_NONE;
		hw_ctrl->proc_mode = HW_CE_PROC_AUTO;
		memset(&(hw_ctrl->grp_ctx), 0, sizeof(HW_CE_SHA_GRP_CTX_S));
		memset(&(hw_ctrl->mgr_ctx), 0, sizeof(SW_CE_SHA_MGR_CTX_S));
		
        //hw_ctrl->key_slot = app_ctrl->sha_para.key_slot;
		break;
	default:
		return CE_SHA_ALGO_MODE_ERROR;
	}

	switch(app_ctrl->sha_para.chan_num)
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
		hw_ctrl->chan_num = HW_CE_CHANNEL_3;
		break;
	default:
		return CE_INVALID_CHANNEL;
	}
	
    if (app_ctrl->sha_para.key_size > 0) {
        memcpy(hw_ctrl->hmac_key, app_ctrl->sha_para.hmac_key, app_ctrl->sha_para.key_size);
        hw_ctrl->key_size = app_ctrl->sha_para.key_size;
    }
	
	return CE_SUCCESS;
}

mt_s32 drv_ce_sha_create(mt_session *p_session, void *p_priv)
{
	drv_ce_check_private(p_priv);

	*p_session = (mt_session)kmalloc(sizeof(HW_CE_SHA_CTRL_S), GFP_KERNEL);

	if(0 == *p_session)
	{
		return CE_GET_HANDLE_FAILED;
	}

	memset((void *)(*p_session), 0, sizeof(HW_CE_SHA_CTRL_S));
	((HW_CE_SHA_CTRL_S *)(*p_session))->priv = p_priv;
	((HW_CE_SHA_CTRL_S *)(*p_session))->chan_num = CE_INVALID_CHANNEL;

	return CE_SUCCESS;
}

mt_s32 drv_ce_sha_destroy(mt_session session)
{
	if(0 == session)
	{
		return CE_INVALID_HANDLE;
	}

	drv_ce_check_keep(session, 2);
	
	kfree((void *)session);
	
	return CE_SUCCESS;
}

mt_s32 drv_ce_sha_init(mt_session session, MT_CE_SHA_CTRL_S *p_ctrl)
{
	HW_CE_SHA_CTRL_S *sha_ce = (HW_CE_SHA_CTRL_S *)session;
	mt_s32 ret = CE_SUCCESS;
	
	if(0 == session)
	{
		return CE_INVALID_HANDLE;
	}

	ret = hw_ce_sha_config_data_prepare(p_ctrl, sha_ce);

	if(CE_SUCCESS == ret)
	{
		drv_ce_channel_lock(session);

		drv_ce_check_keep(session, 0);
		
		drv_ce_channel_unlock(session);
	}

	return ret;
}

mt_s32 drv_ce_sha_get_attr(mt_session session, MT_CE_SHA_CTRL_S *p_attr)
{
	HW_CE_SHA_CTRL_S *sha_ce = (HW_CE_SHA_CTRL_S *)session;
	
	if (0 == session)
		return CE_INVALID_HANDLE;

    if (p_attr == NULL)
        return CE_BAD_PARAMETERS;

    if (sha_ce->key_size > 0) {
        p_attr->sha_para.key_size = sha_ce->key_size;
        memcpy(p_attr->sha_para.hmac_key, sha_ce->hmac_key, sha_ce->key_size); 
    }
    switch (sha_ce->algo_mode) {
        case HW_CE_HASH_ALGO_MODE_SHA1:
            p_attr->sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_SHA1;
            break;
        case HW_CE_HASH_ALGO_MODE_SHA256:
            p_attr->sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_SHA256;
        default:
            break;
    }
    p_attr->sha_para.chan_num = sha_ce->chan_num;

    return CE_SUCCESS;
}

mt_s32 drv_ce_sha_update(mt_session session, mt_u8 is_phy_addr, mt_u8 *p_msg, mt_u32 length)
{
	HW_CE_SHA_CTRL_S *sha_ce = (HW_CE_SHA_CTRL_S *)session;
	mt_s32 ret = CE_SUCCESS;
	mt_u32 cut_length;
	mt_u32 rem_length;
	mt_u32 payload_length;
	mt_u8 *k_msg = NULL;
	mt_u8 *user_msg = NULL;
    mt_u8 *user_data = NULL;//only for kfree
	
	if(0 == session)
	{
		return CE_INVALID_HANDLE;
	}

	drv_ce_channel_lock(session);

	if(drv_ce_check_keep(session, 1))
	{
		#if 0
		//=================================================
		if (HW_CE_HASH_ALGO_MODE_HMAC_SHA256 == sha_ce->algo_mode)
		{
			drv_ce_kt_lock(sha_ce->priv);
			//hw_ce_kt_write_attribute(sha_ce->key_slot, 0x001C03ff);
			hw_ce_kt_write_attribute(sha_ce->key_slot, 0x001000C0);
			hw_ce_kt_write_key(sha_ce->key_slot, sha_ce->hmac_key, HW_KT_SLOT_16B_SIZE);
			hw_ce_kt_slot_active(sha_ce->key_slot, 1);
			drv_ce_kt_unlock(sha_ce->priv);
		}
		//=================================================
		#endif
		hw_ce_sha_init(sha_ce);
	}

	if (HW_CE_HASH_ALGO_MODE_SHA1 == sha_ce->algo_mode)
	{
        //concerto_flush_dcache_by_vaddr(p_msg, length);
		sha_ce->grp_mode = HW_CE_HASH_GRP_NONE;
        if (is_phy_addr) {
            sha_ce->proc_mode = HW_CE_PROC_DMA;
            user_msg = p_msg;
        } else {
            sha_ce->proc_mode = HW_CE_PROC_CPU;
            /* get user data */
            user_msg = (mt_u8 *)drv_ce_copy_from_user_data(p_msg, length);
        }

        if (user_msg)
            ret = hw_ce_sha_update(sha_ce, user_msg, length);

        if (!is_phy_addr) {
            if (user_msg)
                kfree(user_msg);
        }
    }
	else
	{
        if (is_phy_addr) {
			sha_ce->proc_mode = HW_CE_PROC_DMA;
            /* get kernel virtual address */
            k_msg = (mt_u8*)phys_to_virt((unsigned long int)p_msg);
            /* get physical address, for dma operation */
            user_msg = p_msg;
        } else {
			sha_ce->proc_mode = HW_CE_PROC_CPU;
            user_data= (mt_u8*)drv_ce_copy_from_user_data(p_msg, length);
            k_msg = user_data;
            if (k_msg)
                user_msg = k_msg;
            else {
                drv_ce_channel_unlock(session);
                return -1; 
            }
            //dump("kdump", k_msg, length);
        }
		
		cut_length = 128 - sha_ce->mgr_ctx.rem_length;
		cut_length = (length > cut_length) ? cut_length : length;

		memcpy(sha_ce->mgr_ctx.rem_data+sha_ce->mgr_ctx.rem_length, k_msg, cut_length);
		sha_ce->mgr_ctx.rem_length += cut_length;

        //dump("remain", sha_ce->mgr_ctx.rem_data, sha_ce->mgr_ctx.rem_length);

		length -= cut_length;
        /* k_msg is always the kernel virt address */
		k_msg += cut_length;
        /* for DMA, user_msg is phy-addr, for CPU, user_msg is vir-addr */
        user_msg += cut_length;

		if (length > 0 && 128 == sha_ce->mgr_ctx.rem_length)
		{
			if (sha_ce->grp_mode == HW_CE_HASH_GRP_NONE)
			{
				sha_ce->grp_mode = HW_CE_HASH_GRP_FIRST;	 		
			}
			else if (sha_ce->grp_mode == HW_CE_HASH_GRP_FIRST)
			{
				sha_ce->grp_mode = HW_CE_HASH_GRP_MID;
			}
			else if (sha_ce->grp_mode == HW_CE_HASH_GRP_MID)
			{
				sha_ce->grp_mode = HW_CE_HASH_GRP_MID;
			}
			else
			{
                //printk("it is impossible in hal_ce_sha_update!\n");
			}
            /* always use cpu mode to update this 128 bytes */
            sha_ce->proc_mode = HW_CE_PROC_CPU;
			ret = hw_ce_sha_update(sha_ce, sha_ce->mgr_ctx.rem_data, 128);
		}

		if (length>0 && CE_SUCCESS==ret)
		{
			rem_length = (length&0x7F)?(length&0x7F):128;
			payload_length = length-rem_length;

			if (payload_length)
			{
				sha_ce->grp_mode = HW_CE_HASH_GRP_MID;
                if (is_phy_addr)
                    sha_ce->proc_mode = HW_CE_PROC_DMA;
                else
                    sha_ce->proc_mode = HW_CE_PROC_CPU;

				ret = hw_ce_sha_update(sha_ce, user_msg, payload_length);
			}
			memcpy(sha_ce->mgr_ctx.rem_data, (k_msg+payload_length), rem_length);
			sha_ce->mgr_ctx.rem_length = rem_length;
		}

        if (!is_phy_addr) {
            if (user_data) {
                kfree(user_data);
            }
        }
	}

	drv_ce_channel_unlock(session);

	return ret;
}

mt_s32 drv_ce_sha_final(mt_session session, u8 *p_dgst)
{
	HW_CE_SHA_CTRL_S *sha_ce = (HW_CE_SHA_CTRL_S *)session;
	mt_s32 ret;
	
	if(0 == session)
	{
		return CE_INVALID_HANDLE;
	}
	
	drv_ce_channel_lock(session);

	if (HW_CE_HASH_ALGO_MODE_SHA1 == sha_ce->algo_mode)
	{
		ret = hw_ce_sha_final(sha_ce, p_dgst);
	}
	else
	{
		if(sha_ce->grp_mode != HW_CE_HASH_GRP_NONE)
		{
			sha_ce->grp_mode = HW_CE_HASH_GRP_LAST;
		}

        /* buffer of this final update is always <= 128 bytes, use CPU mode */ 
        sha_ce->proc_mode = HW_CE_PROC_CPU;
		ret = hw_ce_sha_update(sha_ce, sha_ce->mgr_ctx.rem_data, sha_ce->mgr_ctx.rem_length);

		if (CE_SUCCESS == ret)
		{
			ret = hw_ce_sha_final(sha_ce, p_dgst);
            //dump("sha256-dgst", p_dgst, 32);
		}
	}
	
	drv_ce_channel_unlock(session);
	
	return ret;
}

#if 1
mt_s32 drv_ce_sha_update_start(mt_session session, u8 *p_msg, u32 length)
{
	HW_CE_SHA_CTRL_S *sha_ce = (HW_CE_SHA_CTRL_S *)session;
	mt_s32 ret = CE_SUCCESS;
	u32 cut_length;
	u32 rem_length;
	u32 payload_length;
	
	if(0 == session)
	{
		return CE_INVALID_HANDLE;
	}

	drv_ce_channel_lock(session);

	if(drv_ce_check_keep(session, 1))
	{
		//=================================================
		if (HW_CE_HASH_ALGO_MODE_HMAC_SHA256 == sha_ce->algo_mode)
		{
			#if 0
			drv_ce_kt_lock(sha_ce->priv);
			//hw_ce_kt_write_attribute(sha_ce->key_slot, 0x001C03ff);
			hw_ce_kt_write_attribute(sha_ce->key_slot, 0x001000C0);
			hw_ce_kt_write_key(sha_ce->key_slot, sha_ce->hmac_key, HW_KT_SLOT_16B_SIZE);
			hw_ce_kt_slot_active(sha_ce->key_slot, 1);
			drv_ce_kt_unlock(sha_ce->priv);
			#endif
		}
		//=================================================
		hw_ce_sha_init(sha_ce);
	}

	if (HW_CE_HASH_ALGO_MODE_SHA1 == sha_ce->algo_mode)
	{
		concerto_flush_dcache_by_vaddr(p_msg, length);
		sha_ce->grp_mode = HW_CE_HASH_GRP_NONE;
		sha_ce->proc_mode = HW_CE_PROC_AUTO;
		ret = hw_ce_sha_update_start(sha_ce, p_msg, length);
	}
	else
	{
		cut_length = 128 - sha_ce->mgr_ctx.rem_length;
		cut_length = (length > cut_length) ? cut_length: length;

		concerto_flush_dcache_by_vaddr(p_msg, length);
		memcpy(sha_ce->mgr_ctx.rem_data+sha_ce->mgr_ctx.rem_length, p_msg, cut_length);
		concerto_flush_dcache_by_vaddr(sha_ce->mgr_ctx.rem_data, 128);
		sha_ce->mgr_ctx.rem_length += cut_length;

		if (128 == sha_ce->mgr_ctx.rem_length)
		{
			if (sha_ce->grp_mode == HW_CE_HASH_GRP_NONE)
			{
				sha_ce->grp_mode = HW_CE_HASH_GRP_FIRST;	 		
			}
			else if (sha_ce->grp_mode == HW_CE_HASH_GRP_FIRST)
			{
				sha_ce->grp_mode = HW_CE_HASH_GRP_MID;
			}
			else if (sha_ce->grp_mode == HW_CE_HASH_GRP_MID)
			{
				sha_ce->grp_mode = HW_CE_HASH_GRP_MID;
			}
			else
			{
                //printk("it is impossible in hal_ce_sha_update!\n");
			}
			sha_ce->proc_mode = HW_CE_PROC_AUTO;
			ret = hw_ce_sha_update(sha_ce, sha_ce->mgr_ctx.rem_data, 128);
		}

		length -= cut_length;
		p_msg += cut_length;

		if (length>0 && CE_SUCCESS==ret)
		{
			rem_length = (length&0x7F)?(length&0x7F):128;
			payload_length = length-rem_length;

			if (payload_length)
			{
				sha_ce->grp_mode = HW_CE_HASH_GRP_MID;
				sha_ce->proc_mode = HW_CE_PROC_AUTO;
				ret = hw_ce_sha_update_start(sha_ce, p_msg, payload_length);
			}
			memcpy(sha_ce->mgr_ctx.rem_data, p_msg+payload_length, rem_length);
			concerto_flush_dcache_by_vaddr(sha_ce->mgr_ctx.rem_data, 128);
			sha_ce->mgr_ctx.rem_length = rem_length;
		}
	}

	return ret;
}

mt_s32 drv_ce_sha_update_polling(mt_session session, u32 time_out)
{
	HW_CE_SHA_CTRL_S *sha_ce = (HW_CE_SHA_CTRL_S *)session;
	
	if(0 == session)
	{
		return CE_INVALID_HANDLE;
	}

	return hw_ce_sha_update_polling(sha_ce, time_out);
}

mt_s32 drv_ce_sha_update_stop(mt_session session)
{
	HW_CE_SHA_CTRL_S *sha_ce = (HW_CE_SHA_CTRL_S *)session;
	
	if(0 == session)
	{
		return CE_INVALID_HANDLE;
	}

	hw_ce_sha_update_polling(sha_ce, 0xFFFFFFFE);
	hw_ce_sha_update_stop(sha_ce);
	
	drv_ce_channel_unlock(session);

	return CE_SUCCESS;
}
#endif
