/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "drv_ce_if.h"
#include "drv_pka.h"
#include "pka_sem.h"
#include "pka_mod.h"

#define MT_ALIGNED(x) (!((uint32_t)x & 0x3))

void drv_ce_check_private(void *priv);
void drv_ce_ecc_lock(mt_session session);
void drv_ce_ecc_unlock(mt_session session);

mt_s32 drv_ce_BN_create(mt_session *p_session, void *p_priv)
{
    drv_ce_check_private(p_priv);

    *p_session = (mt_session)kmalloc(sizeof(DRV_PKA_CTRL_S), GFP_KERNEL);

    if(0 == *p_session)
    {
        return CE_GET_HANDLE_FAILED;
    }

    memset((void *)(*p_session), 0, sizeof(DRV_PKA_CTRL_S));
    ((DRV_PKA_CTRL_S *)(*p_session))->priv = p_priv;

    return CE_SUCCESS;
}

mt_s32 drv_ce_BN_destroy(mt_session session)
{
    if(0 == session)
    {
        return CE_INVALID_HANDLE;
    }

    kfree((void *)session);

    return CE_SUCCESS;
}

mt_s32 drv_ce_BN_mod_mod(mt_session session, mt_u8 *r,
    mt_u8 *d, mt_u8 *m, mt_u32 d_length, mt_u32 m_length)
{
    mt_s32 ret = CE_SUCCESS;

    if(session == 0)
        return CE_INVALID_HANDLE;

    if (r == NULL || d == NULL || m == NULL)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(d) || !MT_ALIGNED(m) || !MT_ALIGNED(r))
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(m_length))
        return CE_BAD_PARAMETERS;

    if (d_length != m_length)
        return MT_FAILURE;

    drv_ce_ecc_lock(session);

    pka_sem_wait(sem_cpu_id, PKA_SEM_REQ_TIME_MAXIMUM);
    ret = pka_mod_operation(PKA_MOD_CMD_MOD, m_length, m, d, NULL, r);
    pka_sem_post();

    drv_ce_ecc_unlock(session);

    return ret;
}

mt_s32 drv_ce_BN_mod_mul(mt_session session, mt_u8 *r,
    mt_u8 *a, mt_u8 *b, mt_u8 *m, mt_u32 a_length, mt_u32 b_length, mt_u32 m_length)
{
    mt_s32 ret = CE_SUCCESS;

    if(session == 0)
        return CE_INVALID_HANDLE;

    if (r == NULL || a == NULL || b == NULL || m == NULL)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(a) || !MT_ALIGNED(b) || !MT_ALIGNED(m) || !MT_ALIGNED(r))
        return CE_BAD_PARAMETERS;

    if (m_length > 256)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(m_length))
        return CE_BAD_PARAMETERS;

    if (a_length!=m_length || b_length!=m_length)
        return CE_BAD_PARAMETERS;

    drv_ce_ecc_lock(session);

    pka_sem_wait(sem_cpu_id, PKA_SEM_REQ_TIME_MAXIMUM);
    ret = pka_mod_operation(PKA_MOD_CMD_MOD_MUL, m_length, m, a, b, r);
    pka_sem_post();

    drv_ce_ecc_unlock(session);

    return ret;
}

mt_s32 drv_ce_BN_mod_add(mt_session session, mt_u8 *r,
    mt_u8 *a, mt_u8 *b, mt_u8 *m, mt_u32 a_length, mt_u32 b_length, mt_u32 m_length)
{
    mt_s32 ret = CE_SUCCESS;

    if(session == 0)
        return CE_INVALID_HANDLE;

    if (r == NULL || a == NULL || b == NULL || m == NULL)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(a) || !MT_ALIGNED(b) || !MT_ALIGNED(m) || !MT_ALIGNED(r))
        return CE_BAD_PARAMETERS;

    if (m_length > 256)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(m_length))
        return CE_BAD_PARAMETERS;

    if (a_length!=m_length || b_length!=m_length)
        return CE_BAD_PARAMETERS;

    drv_ce_ecc_lock(session);

    pka_sem_wait(sem_cpu_id, PKA_SEM_REQ_TIME_MAXIMUM);
    ret = pka_mod_operation(PKA_MOD_CMD_MOD_ADD, m_length, m, a, b, r);
    pka_sem_post();

    drv_ce_ecc_unlock(session);

    return ret;
}

mt_s32 drv_ce_BN_mod_sub(mt_session session, mt_u8 *r,
    mt_u8 *a, mt_u8 *b, mt_u8 *m, mt_u32 a_length, mt_u32 b_length, mt_u32 m_length)
{
    mt_s32 ret = CE_SUCCESS;

    if(session == 0)
        return CE_INVALID_HANDLE;

    if (r == NULL || a == NULL || b == NULL || m == NULL)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(a) || !MT_ALIGNED(b) || !MT_ALIGNED(m) || !MT_ALIGNED(r))
        return CE_BAD_PARAMETERS;

    if (m_length > 256)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(m_length))
        return CE_BAD_PARAMETERS;

    if (a_length!=m_length || b_length!=m_length)
        return CE_BAD_PARAMETERS;

    drv_ce_ecc_lock(session);

    pka_sem_wait(sem_cpu_id, PKA_SEM_REQ_TIME_MAXIMUM);
    ret = pka_mod_operation(PKA_MOD_CMD_MOD_SUB, m_length, m, a, b, r);
    pka_sem_post();

    drv_ce_ecc_unlock(session);

    return ret;
}

mt_s32 drv_ce_BN_mod_inverse(mt_session session, mt_u8 *r,
    mt_u8 *d, mt_u8 *m, mt_u32 d_length, mt_u32 m_length)
{
    mt_s32 ret = CE_SUCCESS;

    if(session == 0)
        return CE_INVALID_HANDLE;

    if (r == NULL || d == NULL || m == NULL)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(d) || !MT_ALIGNED(m) || !MT_ALIGNED(r))
        return CE_BAD_PARAMETERS;

    if (m_length >= 256)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(m_length))
        return CE_BAD_PARAMETERS;

    if (d_length!=m_length)
        return CE_BAD_PARAMETERS;

    drv_ce_ecc_lock(session);

    pka_sem_wait(sem_cpu_id, PKA_SEM_REQ_TIME_MAXIMUM);
    ret = pka_mod_operation(PKA_MOD_CMD_MOD_INV, m_length, m, d, NULL, r);
    pka_sem_post();

    drv_ce_ecc_unlock(session);

    return ret;
}

mt_s32 drv_ce_BN_mod_exp(mt_session session, mt_u8 *r,
    mt_u8 *a, mt_u8 *p, mt_u8 *m, mt_u32 a_length, mt_u32 p_length, mt_u32 m_length)
{
    mt_s32 ret = CE_SUCCESS;

    if(session == 0)
        return CE_INVALID_HANDLE;

    if (r == NULL || a == NULL || p == NULL || m == NULL)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(a) || !MT_ALIGNED(p) || !MT_ALIGNED(m) || !MT_ALIGNED(r))
        return CE_BAD_PARAMETERS;

    if (m_length > 256)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(m_length))
        return CE_BAD_PARAMETERS;

    if (a_length!=m_length || p_length!=m_length)
        return MT_FAILURE;

    drv_ce_ecc_lock(session);

    pka_sem_wait(sem_cpu_id, PKA_SEM_REQ_TIME_MAXIMUM);
    ret = pka_mod_operation(PKA_MOD_CMD_MOD_POW, m_length, m, a, p, r);
    pka_sem_post();

    drv_ce_ecc_unlock(session);

    return ret;
}
