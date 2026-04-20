/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "drv_ce_if.h"
#include "hw_ce_if.h"

void drv_ce_check_private(void *priv);
void drv_ce_rsa_lock(mt_session session);
void drv_ce_rsa_unlock(mt_session session);


mt_s32 drv_ce_rsa_create(mt_session *p_session, void *p_priv)
{
	drv_ce_check_private(p_priv);

	*p_session = (mt_session)kmalloc(sizeof(DRV_PKA_RSA_CTRL_S), GFP_KERNEL);

	if(0 == *p_session)
	{
		return CE_GET_HANDLE_FAILED;
	}

	memset((void *)(*p_session), 0, sizeof(DRV_PKA_RSA_CTRL_S));
	((DRV_PKA_RSA_CTRL_S *)(*p_session))->priv = p_priv;

	return CE_SUCCESS;
}

mt_s32 drv_ce_rsa_destroy(mt_session session)
{
	if (0 == session) {
		return CE_INVALID_HANDLE;
	}

	kfree((void *)session);
	
	return CE_SUCCESS;
}

/*
static void dump_data(unsigned char *data_src, mt_u32 len)
{
    unsigned int i = 0;

    printk("\n");
    for (i = 0; i < len; i++) {
        printk("%02x ", data_src[i]);
        if ((i+1) % 16 == 0) {
            printk("\n");
        }
    }
    printk("\n");

}
*/

mt_s32 drv_ce_rsa_config(mt_session session, MT_CE_RSA_CTRL_S *p_ctrl)
{
    mt_s32 ret = CE_SUCCESS;
    DRV_PKA_RSA_CTRL_S *rsa_ce = NULL;

    if (0 == session) {
        return CE_INVALID_HANDLE;
    }

    if (p_ctrl == NULL) {
        return CE_BAD_PARAMETERS;
    }

    rsa_ce = (DRV_PKA_RSA_CTRL_S *)session;
    rsa_ce->p_m = p_ctrl->rsa_para.p_m;
    rsa_ce->p_e = p_ctrl->rsa_para.p_e;
    rsa_ce->key_length = p_ctrl->rsa_para.key_length;
    rsa_ce->exp_length = p_ctrl->rsa_para.exp_length;

    return ret;

}

mt_s32 drv_ce_rsa_process(mt_session session, mt_u8 *p_src_addr, mt_u8 *p_dst_addr, mt_u32 src_length)
{
    mt_s32 ret = CE_SUCCESS;
    DRV_PKA_RSA_CTRL_S *rsa_ce = NULL;
    mt_u32 *user_in = (mt_u32 *)p_src_addr;
    mt_u32 *user_out = (mt_u32 *)p_dst_addr;

    if (0 == session) {
        return CE_INVALID_HANDLE;
    }

    /* Only support RSA1024, RSA2048 */
    if (src_length < 128 || src_length > 256) {
        return CE_NOT_SUPPORT;
    }

    drv_ce_rsa_lock(session);

    rsa_ce = (DRV_PKA_RSA_CTRL_S *)session;

    ret = hw_ce_rsa_process((mt_u32 *)rsa_ce->p_m, rsa_ce->key_length, (mt_u32 *)rsa_ce->p_e, rsa_ce->exp_length, 
            user_in, user_out, src_length);

    drv_ce_rsa_unlock(session);

    return ret;
}

