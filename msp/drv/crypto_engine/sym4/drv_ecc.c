/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "drv_ce_if.h"
#include "drv_pka.h"
#include "pka_sem.h"
#include "pka_ecc.h"

#define MT_ALIGNED(x) (!((uint32_t)x & 0x3))

void drv_ce_check_private(void *priv);
void drv_ce_ecc_lock(mt_session session);
void drv_ce_ecc_unlock(mt_session session);
	
mt_s32 drv_ce_EC_POINT_create(mt_session *p_session, void *p_priv)
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

mt_s32 drv_ce_EC_POINT_destroy(mt_session session)
{
    if(0 == session)
    {
        return CE_INVALID_HANDLE;
    }

    kfree((void *)session);

    return CE_SUCCESS;
}

mt_s32 drv_ce_EC_POINT_mul(mt_session session, MT_CE_EC_PARAMS_S xParams,
    MT_CE_EC_POINT_S *r, const u8 *g_scalar,  MT_CE_EC_POINT_S *p_point, const u8 *p_scalar)
{
    mt_s32 ret = CE_SUCCESS;
    MT_CE_EC_POINT_S point;
    const u8 *scalar;

    if(session == 0)
        return CE_INVALID_HANDLE;

    if (r == NULL)
        return CE_BAD_PARAMETERS;

    if (r->X == NULL || r->Y == NULL)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(r->X) || !MT_ALIGNED(r->Y))
        return CE_BAD_PARAMETERS;

    if (xParams.q == NULL || xParams.a == NULL)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(xParams.q) || !MT_ALIGNED(xParams.a))
        return CE_BAD_PARAMETERS;

    if (g_scalar && xParams.GX && xParams.GY) {
        point.X = xParams.GX;
        point.Y = xParams.GY;
        scalar = g_scalar;
    } else if (p_scalar && p_point && p_point->X && p_point->Y) {
        point.X = p_point->X;
        point.Y = p_point->Y;
        scalar = p_scalar;
    } else {
        return CE_BAD_PARAMETERS;
    }

    if (!MT_ALIGNED(point.X) || !MT_ALIGNED(point.Y))
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(scalar))
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(xParams.keySize))
        return CE_BAD_PARAMETERS;

    drv_ce_ecc_lock(session);

    pka_sem_wait(sem_cpu_id, PKA_SEM_REQ_TIME_MAXIMUM);
    ret = pka_ecc_point_mul(xParams.keySize, xParams.q, xParams.a,
                        point.X, point.Y, scalar, r->X, r->Y);

    pka_sem_post();

    drv_ce_ecc_unlock(session);

    return ret;
}

mt_s32 drv_ce_EC_POINT_add(mt_session session, MT_CE_EC_PARAMS_S xParams,
    MT_CE_EC_POINT_S *r, const MT_CE_EC_POINT_S *a, const MT_CE_EC_POINT_S *b)
{
    mt_s32 ret = CE_SUCCESS;

    if(session == 0)
        return CE_INVALID_HANDLE;

    if (r == NULL || a == NULL || b == NULL)
        return CE_BAD_PARAMETERS;

    if (r->X == NULL || r->Y == NULL)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(r->X) || !MT_ALIGNED(r->Y))
        return CE_BAD_PARAMETERS;

    if (a->X == NULL || a->Y == NULL)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(a->X) || !MT_ALIGNED(a->Y))
        return CE_BAD_PARAMETERS;

    if (b->X == NULL || b->Y == NULL)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(b->X) || !MT_ALIGNED(b->Y))
        return CE_BAD_PARAMETERS;

    if (xParams.q == NULL || xParams.a == NULL)
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(xParams.q) || !MT_ALIGNED(xParams.a))
        return CE_BAD_PARAMETERS;

    if (!MT_ALIGNED(xParams.keySize))
        return CE_BAD_PARAMETERS;

    drv_ce_ecc_lock(session);

    pka_sem_wait(sem_cpu_id, PKA_SEM_REQ_TIME_MAXIMUM);
    ret = pka_ecc_point_add(xParams.keySize, xParams.q, xParams.a,
                        a->X, a->Y, b->X, b->Y, r->X, r->Y);
    pka_sem_post();

    drv_ce_ecc_unlock(session);

    return ret;
}

mt_s32 drv_ce_reset(mt_u32 mode)
{
    return CE_SUCCESS;
}

