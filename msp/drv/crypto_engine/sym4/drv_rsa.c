/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "drv_ce_if.h"
#include "drv_pka.h"
#include "pka_sem.h"
#include "pka_rsa.h"

#define MT_ALIGNED(x) (!((uint32_t)x & 0x3))

void drv_ce_check_private(void *priv);
void drv_ce_ecc_lock(mt_session session);
void drv_ce_ecc_unlock(mt_session session);

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
    DRV_PKA_RSA_CTRL_S *rsa_ctrl;

    if(session == 0)
        return CE_INVALID_HANDLE;

    rsa_ctrl = (DRV_PKA_RSA_CTRL_S *)session;

    kfree((void *)session);

    return CE_SUCCESS;
}

mt_s32 drv_ce_rsa_config(mt_session session, MT_CE_RSA_CTRL_S *p_ctrl)
{
    mt_s32 ret = CE_SUCCESS;
    DRV_PKA_RSA_CTRL_S *rsa_ctrl = NULL;
    mt_u32 key_size;

    if(session == 0)
        return CE_INVALID_HANDLE;

    if (p_ctrl == NULL)
        return CE_BAD_PARAMETERS;

    key_size = p_ctrl->rsa_para.key_length;
    if (key_size < PKA_RSA_KEY_1024 || key_size > PKA_RSA_KEY_2048)
        return CE_BAD_PARAMETERS;

    if (p_ctrl->rsa_para.exp_length != key_size)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(p_ctrl->rsa_para.p_m) || !MT_ALIGNED(p_ctrl->rsa_para.p_e))
        return CE_BAD_PARAMETERS;

    rsa_ctrl = (DRV_PKA_RSA_CTRL_S *)session;

    rsa_ctrl->p_m = p_ctrl->rsa_para.p_m;
    rsa_ctrl->p_e = p_ctrl->rsa_para.p_e;
    rsa_ctrl->key_length = key_size;
    rsa_ctrl->exp_length = key_size;

    return ret;
}

mt_s32 drv_ce_rsa_process(mt_session session, mt_u8 *p_src_addr, mt_u8 *p_dst_addr, mt_u32 src_length)
{
    mt_s32 ret = CE_SUCCESS;
    DRV_PKA_RSA_CTRL_S *rsa_ctrl = NULL;
    mt_u8 *inbuf = NULL;
    mt_u8 *input = NULL;

    if(session == 0)
        return CE_INVALID_HANDLE;

    if (p_src_addr == NULL || p_dst_addr == NULL)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(p_src_addr) || !MT_ALIGNED(p_dst_addr))
        return CE_BAD_PARAMETERS;

    rsa_ctrl = (DRV_PKA_RSA_CTRL_S *)session;

    if (src_length != rsa_ctrl->key_length) {
        inbuf = kmalloc(rsa_ctrl->key_length, GFP_KERNEL);
        if (inbuf == NULL) {
            ret = CE_RSA_PROCESS_FAILED;
            goto EXIT;
        }

        input = inbuf;
        memset(input, 0, rsa_ctrl->key_length);
        memcpy(input+rsa_ctrl->key_length-src_length, p_src_addr, src_length);
    } else
        input = p_src_addr;

    drv_ce_ecc_lock(session);

    pka_sem_wait(sem_cpu_id, PKA_SEM_REQ_TIME_MAXIMUM);
    ret = pka_rsa_operation(rsa_ctrl->key_length, rsa_ctrl->p_m, rsa_ctrl->p_e,
        p_dst_addr, input);

    pka_sem_post();

    drv_ce_ecc_unlock(session);

EXIT:
    if (inbuf) {
        kfree(inbuf);
        inbuf = NULL;
    }

    return ret;
}

