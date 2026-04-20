/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "drv_ce_if.h"
#include "hw_ce_if.h"

void drv_ce_check_private(void *priv);
void drv_ce_ecc_lock(mt_session session);
void drv_ce_ecc_unlock(mt_session session);
void drv_ce_rsa_lock(mt_session session);
void drv_ce_rsa_unlock(mt_session session);
	
mt_s32 drv_ce_BN_create(mt_session *p_session, void *p_priv)
{
	drv_ce_check_private(p_priv);
	
	*p_session = (mt_session)kmalloc(sizeof(HW_BN_OP_S), GFP_KERNEL);

	if(0 == *p_session)
	{
		return CE_GET_HANDLE_FAILED;
	}

	memset((void *)(*p_session), 0, sizeof(HW_BN_OP_S));
	((HW_BN_OP_S *)(*p_session))->priv = p_priv;
	
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

mt_s32 drv_ce_BN_mod_mod(mt_session session, mt_u8 *r, mt_u8 *d, mt_u8 *m, mt_u32 d_length, mt_u32 m_length)
{
    HW_BN_OP_S *op = (HW_BN_OP_S *)session;
    mt_s32 ret = CE_SUCCESS;

    if(0 == session)
    {
        return CE_INVALID_HANDLE;
    }

    if (d_length&0x03 || m_length&0x03)
    {
        return CE_BN_MOD_LENGTH_ERROR;
    }
    
    if ( (mt_u32)d & 0x3)
    {
        op->a = kmalloc(d_length, GFP_KERNEL);
        memcpy(op->a, d, d_length);
    }
    else
    {
        op->a = d;
    }

    //op->b = kmalloc(d_length, GFP_KERNEL);
    //memset(op->b, 0, d_length);

    if ( (mt_u32)m & 0x3)
    {
        op->m = kmalloc(m_length, GFP_KERNEL);
        memcpy(op->m, m, m_length);
    }
    else
    {
        op->m = m;
    }
	
    if ( (mt_u32)r & 0x3)
    {
        op->r = kmalloc(m_length, GFP_KERNEL);
    }
    else
    {
        op->r = r;
    }

    op->a_length = d_length;
    //op->b_length = d_length;
    op->m_length = m_length;

    if ( (d_length<=m_length) || (d_length-m_length==1) )
    {
    drv_ce_ecc_lock(session);
    ret = hw_bn_mod(HW_MOD_MOD, op);
    drv_ce_ecc_unlock(session);
    }
    else
    {
    ret = -408;
    }

    if ( (mt_u32)d & 0x3)
    {
        kfree(op->a);
    }

    //kfree(op->b);

    if ( (mt_u32)m & 0x3)
    {
        kfree(op->m);
    }

    if ( (mt_u32)r & 0x3)
    {
        memcpy(r, op->r, m_length);
        kfree(op->r);
    }

    return ret;
}

mt_s32 drv_ce_BN_mod_mul(mt_session session, u8 *r, u8 *a, u8 *b, u8 *m, u32 a_length, u32 b_length, u32 m_length)
{
    HW_BN_OP_S *op = (HW_BN_OP_S *)session;
    mt_s32 ret = CE_SUCCESS;

    if(0 == session)
    {
        return CE_INVALID_HANDLE;
    }

    if (a_length&0x03 || b_length&0x03 || m_length&0x03)
    {
        return CE_BN_MOD_LENGTH_ERROR;
    }

    if ( (mt_u32)a & 0x3)
    {
        op->a = kmalloc(a_length, GFP_KERNEL);
        memcpy(op->a, a, a_length);
    }
    else
    {
        op->a = a;
    }

    if ( (mt_u32)b & 0x3)
    {
        op->b = kmalloc(b_length, GFP_KERNEL);
        memcpy(op->b, b, b_length);
    }
    else
    {
        op->b = b;
    }

    if ( (mt_u32)m & 0x3)
    {
        op->m = kmalloc(m_length, GFP_KERNEL);
        memcpy(op->m, m, m_length);
    }
    else
    {
        op->m = m;
    }

    if ( (mt_u32)r & 0x3)
    {
        op->r = kmalloc(m_length, GFP_KERNEL);
    }
    else
    {
        op->r = r;
    }

    op->a_length = a_length;
    op->b_length = b_length;
    op->m_length = m_length;

    drv_ce_ecc_lock(session);
    ret = hw_bn_mod(HW_MOD_MUL, op);
    drv_ce_ecc_unlock(session);

    if ( (mt_u32)a & 0x3)
    {
        kfree(op->a);
    }

    if ( (mt_u32)b & 0x3)
    {
        kfree(op->b);
    }

    if ( (mt_u32)m & 0x3)
    {
        kfree(op->m);
    }

    if ( (mt_u32)r & 0x3)
    {
        memcpy(r, op->r, m_length);
        kfree(op->r);
    }

    return ret;
}

mt_s32 drv_ce_BN_mod_add(mt_session session, u8 *r, u8 *a, u8 *b, u8 *m, u32 a_length, u32 b_length, u32 m_length)
{
    HW_BN_OP_S *op = (HW_BN_OP_S *)session;
    mt_s32 ret = CE_SUCCESS;

    if(0 == session)
    {
        return CE_INVALID_HANDLE;
    }

    if (a_length&0x03 || b_length&0x03 || m_length&0x03)
    {
        return CE_BN_MOD_LENGTH_ERROR;
    }
    
    if ( (mt_u32)a & 0x3)
    {
        op->a = kmalloc(a_length, GFP_KERNEL);
        memcpy(op->a, a, a_length);
    }
    else
    {
        op->a = a;
    }

    if ( (mt_u32)b & 0x3)
    {
        op->b = kmalloc(b_length, GFP_KERNEL);
        memcpy(op->b, b, b_length);
    }
    else
    {
        op->b = b;
    }

    if ( (mt_u32)m & 0x3)
    {
        op->m = kmalloc(m_length, GFP_KERNEL);
        memcpy(op->m, m, m_length);
    }
    else
    {
        op->m = m;
    }

    if ( (mt_u32)r & 0x3)
    {
        op->r = kmalloc(m_length, GFP_KERNEL);
    }
    else
    {
        op->r = r;
    }

    op->a_length = a_length;
    op->b_length = b_length;
    op->m_length = m_length;

    drv_ce_ecc_lock(session);
    ret = hw_bn_mod(HW_MOD_ADD, op);
    drv_ce_ecc_unlock(session);

    if ( (mt_u32)a & 0x3)
    {
        kfree(op->a);
    }

    if ( (mt_u32)b & 0x3)
    {
        kfree(op->b);
    }

    if ( (mt_u32)m & 0x3)
    {
        kfree(op->m);
    }

    if ( (mt_u32)r & 0x3)
    {
        memcpy(r, op->r, m_length);
        kfree(op->r);
    }

    return ret;
}

mt_s32 drv_ce_BN_mod_sub(mt_session session, u8 *r, u8 *a, u8 *b, u8 *m, u32 a_length, u32 b_length, u32 m_length)
{
    HW_BN_OP_S *op = (HW_BN_OP_S *)session;
    mt_s32 ret = CE_SUCCESS;

    if(0 == session)
    {
        return CE_INVALID_HANDLE;
    }

    if (a_length&0x03 || b_length&0x03 || m_length&0x03)
    {
        return CE_BN_MOD_LENGTH_ERROR;
    }
    
    if ( (mt_u32)a & 0x3)
    {
        op->a = kmalloc(a_length, GFP_KERNEL);
        memcpy(op->a, a, a_length);
    }
    else
    {
        op->a = a;
    }

    if ( (mt_u32)b & 0x3)
    {
        op->b = kmalloc(b_length, GFP_KERNEL);
        memcpy(op->b, b, b_length);
    }
    else
    {
        op->b = b;
    }

    if ( (mt_u32)m & 0x3)
    {
        op->m = kmalloc(m_length, GFP_KERNEL);
        memcpy(op->m, m, m_length);
    }
    else
    {
        op->m = m;
    }

    if ( (mt_u32)r & 0x3)
    {
        op->r = kmalloc(m_length, GFP_KERNEL);
    }
    else
    {
        op->r = r;
    }

    op->a_length = a_length;
    op->b_length = b_length;
    op->m_length = m_length;

    drv_ce_ecc_lock(session);
    ret = hw_bn_mod(HW_MOD_SUB, op);
    drv_ce_ecc_unlock(session);

    if ( (mt_u32)a & 0x3)
    {
        kfree(op->a);
    }

    if ( (mt_u32)b & 0x3)
    {
        kfree(op->b);
    }

    if ( (mt_u32)m & 0x3)
    {
        kfree(op->m);
    }

    if ( (mt_u32)r & 0x3)
    {
        memcpy(r, op->r, m_length);
        kfree(op->r);
    }

    return ret;
}

mt_s32 drv_ce_BN_mod_inverse(mt_session session, u8 *r, u8 *d, u8 *m, u32 d_length, u32 m_length)
{
    HW_BN_OP_S *op = (HW_BN_OP_S *)session;
    mt_s32 ret = CE_SUCCESS;

    if(0 == session)
    {
        return CE_INVALID_HANDLE;
    }

    if (d_length&0x03 || m_length&0x03)
    {
        return CE_BN_MOD_LENGTH_ERROR;
    }
    
    if ( (mt_u32)d & 0x3)
    {
        op->a = kmalloc(d_length, GFP_KERNEL);
        memcpy(op->a, d, d_length);
    }
    else
    {
        op->a = d;
    }

    op->b = kmalloc(d_length, GFP_KERNEL);
    memset(op->b, 0, d_length);

    if ( (mt_u32)m & 0x3)
    {
        op->m = kmalloc(m_length, GFP_KERNEL);
        memcpy(op->m, m, m_length);
    }
    else
    {
        op->m = m;
    }

    if ( (mt_u32)r & 0x3)
    {
        op->r = kmalloc(m_length, GFP_KERNEL);
    }
    else
    {
        op->r = r;
    }

    op->a_length = d_length;
    op->b_length = d_length;
    op->m_length = m_length;

    drv_ce_ecc_lock(session);
    ret = hw_bn_mod(HW_MOD_INV, op);
    drv_ce_ecc_unlock(session);

    if ( (mt_u32)d & 0x3)
    {
        kfree(op->a);
    }

    kfree(op->b);

    if ( (mt_u32)m & 0x3)
    {
        kfree(op->m);
    }

    if ( (mt_u32)r & 0x3)
    {
        memcpy(r, op->r, m_length);
        kfree(op->r);
    }

    return ret;
}

mt_s32 drv_ce_BN_mod_exp(mt_session session, u8 *r, u8 *a, u8 *p, u8 *m, u32 a_length, u32 p_length, u32 m_length)
{
    HW_BN_OP_S *op = (HW_BN_OP_S *)session;
    mt_s32 ret = CE_SUCCESS;

    if(0 == session)
    {
        return CE_INVALID_HANDLE;
    }

    if (a_length&0x03 || p_length&0x03 || m_length&0x03)
    {
        return CE_BN_MOD_LENGTH_ERROR;
    }
    
    if ( (mt_u32)a & 0x3)
    {
        op->a = kmalloc(a_length, GFP_KERNEL);
        memcpy(op->a, a, a_length);
    }
    else
    {
        op->a = a;
    }

    if ( (mt_u32)p & 0x3)
    {
        op->b = kmalloc(p_length, GFP_KERNEL);
        memcpy(op->b, p, p_length);
    }
    else
    {
        op->b = p;
    }

    if ( (mt_u32)m & 0x3)
    {
        op->m = kmalloc(m_length, GFP_KERNEL);
        memcpy(op->m, m, m_length);
    }
    else
    {
        op->m = m;
    }

    if ( (mt_u32)r & 0x3)
    {
        op->r = kmalloc(m_length, GFP_KERNEL);
    }
    else
    {
        op->r = r;
    }

    op->a_length = a_length;
    op->b_length = p_length;
    op->m_length = m_length;

    drv_ce_rsa_lock(session);
    ret = hw_bn_mod(HW_MOD_EXP, op);
    drv_ce_rsa_unlock(session);

    if ( (mt_u32)a & 0x3)
    {
        kfree(op->a);
    }

    if ( (mt_u32)p & 0x3)
    {
        kfree(op->b);
    }

    if ( (mt_u32)m & 0x3)
    {
        kfree(op->m);
    }

    if ( (mt_u32)r & 0x3)
    {
        memcpy(r, op->r, m_length);
        kfree(op->r);
    }

    return ret;
}



mt_s32 drv_ce_EC_POINT_create(mt_session *p_session, void *p_priv)
{
	drv_ce_check_private(p_priv);
	
	*p_session = (mt_session)kmalloc(sizeof(HW_EC_OP_S), GFP_KERNEL);

	if(0 == *p_session)
	{
		return CE_GET_HANDLE_FAILED;
	}

	memset((void *)(*p_session), 0, sizeof(HW_EC_OP_S));
	((HW_EC_OP_S *)(*p_session))->priv = p_priv;
	
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

mt_s32 drv_ce_EC_POINT_mul(mt_session session, MT_CE_EC_PARAMS_S xParams, MT_CE_EC_POINT_S *r, const u8 *g_scalar,  MT_CE_EC_POINT_S *p_point, const u8 *p_scalar)
{
	HW_EC_OP_S *op = (HW_EC_OP_S *)session;
	mt_s32 ret = CE_SUCCESS;
	
	if(0 == session)
	{
		return CE_INVALID_HANDLE;
	}

	if( (mt_u32)(xParams.q) & 0x3)
	{
		op->curve_p = kmalloc(xParams.keySize, GFP_KERNEL);
		memcpy(op->curve_p, xParams.q, xParams.keySize);
	}
	else
	{
		op->curve_p = xParams.q;
	}

	if( (mt_u32)(xParams.a) & 0x3)
	{
		op->curve_a = kmalloc(xParams.keySize, GFP_KERNEL);
		memcpy(op->curve_a, xParams.a, xParams.keySize);
	}
	else
	{
		op->curve_a = xParams.a;
	}
	
	if(g_scalar)
	{
		if( (mt_u32)(xParams.GX) & 0x3 )
		{
			op->ec_point_mul.ptX= kmalloc(xParams.keySize, GFP_KERNEL);
			memcpy(op->ec_point_mul.ptX, xParams.GX, xParams.keySize);
		}
		else
		{
			op->ec_point_mul.ptX = xParams.GX;
		}

		if( (mt_u32)(xParams.GY) & 0x3 )
		{
			op->ec_point_mul.ptY= kmalloc(xParams.keySize, GFP_KERNEL);
			memcpy(op->ec_point_mul.ptY, xParams.GY, xParams.keySize);
		}
		else
		{
			op->ec_point_mul.ptY = xParams.GY;
		}

		if( (mt_u32)g_scalar & 0x3 )
		{
			op->ec_point_mul.scaler= kmalloc(xParams.keySize, GFP_KERNEL);
			memcpy(op->ec_point_mul.scaler, g_scalar, xParams.keySize);
		}
		else
		{
			op->ec_point_mul.scaler = (u8 *)g_scalar;
		}
	}
	else if(p_scalar && p_point)
	{
		if ( (mt_u32)(p_point->X) & 0x3 )
		{
			op->ec_point_mul.ptX = kmalloc(xParams.keySize, GFP_KERNEL);
			memcpy(op->ec_point_mul.ptX, p_point->X, xParams.keySize);
		}
		else
		{
			op->ec_point_mul.ptX = p_point->X;
		}

		if ( (mt_u32)(p_point->Y) & 0x3 )
		{
			op->ec_point_mul.ptY = kmalloc(xParams.keySize, GFP_KERNEL);
			memcpy(op->ec_point_mul.ptY, p_point->Y, xParams.keySize);
		}
		else
		{
			op->ec_point_mul.ptY = p_point->Y;
		}

		if ( (mt_u32)p_scalar & 0x3 )
		{
			op->ec_point_mul.scaler = kmalloc(xParams.keySize, GFP_KERNEL);
			memcpy(op->ec_point_mul.scaler, p_scalar, xParams.keySize);
		}
		else
		{
			op->ec_point_mul.scaler = (u8 *)p_scalar;
		}
	}
	else
	{
		return CE_EC_POINT_BAD_PARAMS;
	}
	
	op->length = xParams.keySize;

	if( (mt_u32)(r->X) & 0x3)
	{
		op->rsX = kmalloc(xParams.keySize, GFP_KERNEL);
	}
	else
	{
		op->rsX = r->X;
	}

	if( (mt_u32)(r->Y) & 0x3)
	{
		op->rsY = kmalloc(xParams.keySize, GFP_KERNEL);
	}
	else
	{
		op->rsY = r->Y;
	}

	drv_ce_ecc_lock(session);
	ret = hw_ec_point(HW_POINT_MUL, op);
	drv_ce_ecc_unlock(session);

	if( (mt_u32)(xParams.q) & 0x3)
	{
		kfree(op->curve_p);
	}

	if( (mt_u32)(xParams.a) & 0x3)
	{
		kfree(op->curve_a);
	}
	
	if(g_scalar)
	{
		if( (mt_u32)(xParams.GX) & 0x3 )
		{
			kfree(op->ec_point_mul.ptX);
		}

		if( (mt_u32)(xParams.GY) & 0x3 )
		{
			kfree(op->ec_point_mul.ptY);
		}

		if( (mt_u32)g_scalar & 0x3 )
		{
			kfree(op->ec_point_mul.scaler);
		}
	}
	else if(p_scalar && p_point)
	{
		if ( (mt_u32)(p_point->X) & 0x3 )
		{
			kfree(op->ec_point_mul.ptX);
		}

		if ( (mt_u32)(p_point->Y) & 0x3 )
		{
			kfree(op->ec_point_mul.ptY);
		}

		if ( (mt_u32)p_scalar & 0x3 )
		{
			kfree(op->ec_point_mul.scaler);
		}

	}

	if( (mt_u32)(r->X) & 0x3)
	{
		memcpy(r->X, op->rsX, xParams.keySize);
		kfree(op->rsX);
	}

	if( (mt_u32)(r->Y) & 0x3)
	{
		memcpy(r->Y, op->rsY, xParams.keySize);
		kfree(op->rsY);
	}
	
	return ret;
}

mt_s32 drv_ce_EC_POINT_add(mt_session session, MT_CE_EC_PARAMS_S xParams, MT_CE_EC_POINT_S *r, const MT_CE_EC_POINT_S *a, const MT_CE_EC_POINT_S *b)
{
	HW_EC_OP_S *op = (HW_EC_OP_S *)session;
	mt_s32 ret = CE_SUCCESS;
	
	if(0 == session)
	{
		return CE_INVALID_HANDLE;
	}

	if ( (mt_u32)(xParams.q) & 0x3 )
	{
		op->curve_p = kmalloc(xParams.keySize, GFP_KERNEL);
		memcpy(op->curve_p, xParams.q, xParams.keySize);
	}
	else
	{
		op->curve_p = xParams.q;
	}

	if ( (mt_u32)(xParams.a) & 0x3 )
	{
		op->curve_a = kmalloc(xParams.keySize, GFP_KERNEL);
		memcpy(op->curve_a, xParams.a, xParams.keySize);
	}
	else
	{
		op->curve_a = xParams.a;
	}

	if ((mt_u32)(a->X) & 0x3)
	{
		op->ec_point_add.pt1X = kmalloc(xParams.keySize, GFP_KERNEL);
		memcpy(op->ec_point_add.pt1X, a->X, xParams.keySize);
	}
	else
	{
		op->ec_point_add.pt1X = a->X;
	}

	if ((mt_u32)(a->Y) & 0x3)
	{
		op->ec_point_add.pt1Y = kmalloc(xParams.keySize, GFP_KERNEL);
		memcpy(op->ec_point_add.pt1Y, a->Y, xParams.keySize);
	}
	else
	{
		op->ec_point_add.pt1Y = a->Y;
	}

	if ((mt_u32)(b->X) & 0x3)
	{
		op->ec_point_add.pt2X = kmalloc(xParams.keySize, GFP_KERNEL);
		memcpy(op->ec_point_add.pt2X, b->X, xParams.keySize);
	}
	else
	{
		op->ec_point_add.pt2X = b->X;
	}

	if ((mt_u32)(b->Y) & 0x3)
	{
		op->ec_point_add.pt2Y = kmalloc(xParams.keySize, GFP_KERNEL);
		memcpy(op->ec_point_add.pt2Y, b->Y, xParams.keySize);
	}
	else
	{
		op->ec_point_add.pt2Y = b->Y;
	}

	
	op->length = xParams.keySize;

	if ((mt_u32)(r->X) & 0x3)
	{
		op->rsX = kmalloc(xParams.keySize, GFP_KERNEL);
	}
	else
	{
		op->rsX = r->X;
	}

	if ((mt_u32)(r->Y) & 0x3)
	{
		op->rsY = kmalloc(xParams.keySize, GFP_KERNEL);
	}
	else
	{
		op->rsY = r->Y;
	}

	drv_ce_ecc_lock(session);
	ret = hw_ec_point(HW_POINT_ADD, op);
	drv_ce_ecc_unlock(session);

	if ( (mt_u32)(xParams.q) & 0x3 )
	{
		kfree(op->curve_p);
	}

	if ( (mt_u32)(xParams.a) & 0x3 )
	{
		kfree(op->curve_a);
	}

	if ((mt_u32)(a->X) & 0x3)
	{
		kfree(op->ec_point_add.pt1X);
	}

	if ((mt_u32)(a->Y) & 0x3)
	{
		kfree(op->ec_point_add.pt1Y);
	}

	if ((mt_u32)(b->X) & 0x3)
	{
		kfree(op->ec_point_add.pt2X);
	}

	if ((mt_u32)(b->Y) & 0x3)
	{
		kfree(op->ec_point_add.pt2Y);
	}

	if ((mt_u32)(r->X) & 0x3)
	{
		memcpy(r->X, op->rsX, xParams.keySize);
		kfree(op->rsX);
	}

	if ((mt_u32)(r->Y) & 0x3)
	{
		memcpy(r->Y, op->rsY, xParams.keySize);
		kfree(op->rsY);
	}

	return ret;
}

mt_s32 drv_ce_reset(mt_u32 mode)
{
    return hw_ce_reset(mode);
}
#if 0
static s32 ecdsa_check_zero(u8 *data, u32 size)
{
	u32 i;
	
	for(i=0; i<size; i++)
	{
		if (0 != data[i])
			return 1;
	}

	return 0;
}

static s32 ecdsa_check_interval(MT_CE_EC_PARAMS_S xParams, u8 *k)
{
	u32 len = xParams.keySize;
	u32 i;
	u32 larger_ind = 0;

	for(i=0; i<len; i++)
	{
		if (0 != k[i])
			break;
	}

	if (i >= len)
		return 1;



	u8 *n = mtos_malloc(len);
	memcpy(n, xParams.n, len);
	n[len-1] -= 1;

	for(i=0; i<len; i++)
	{
		if (k[i] < n[i])
		{
			break;
		}
			
		if (k[i] > n[i])
		{
			larger_ind = 1;
			break;
		}
	}

	if(larger_ind)
	{
		mtos_free(n);
		return 2;
	}
	else
	{
		mtos_free(n);
		return 0;
	}
}

RET_CODE drv_SecEcdsaSign(MT_CE_EC_PARAMS_S xParams, MT_CE_SHA_ALGO_MODE_E xHashType, const u8 *pxPrivKey, const u8 *pxMessage, u32 xMessageSize, u8 *pxSigR, u8 *pxSigS)
{
	RET_CODE ret;

	u8 *r = pxSigR;
	u8 *s = pxSigS;
	u8 *n = xParams.n;
	u8 *d = (u8 *)pxPrivKey;
	u8 loop_r_cnt = 0;
	u8 loop_s_cnt = 0;

	u32 len = xParams.keySize;
	u8 e[len];
	u8 x[len];
	u8 y[len];
	u8 kinv[len];
	u8 dr[len];
	u8 e_dr[len];
	//k为随机值或伪随机值
	u8 k[] = {0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88};

	MT_CE_EC_POINT_S kPoint;
	kPoint.X = x;
	kPoint.Y = y;

	// 1. Select a random or pseudorandom integer k, 1 <= k <= n-1.
	if ( ecdsa_check_interval(xParams, k) )
	{
		return CE_ECDSA_SIGN_CHECK_K_FAILED;
	}

	// 5. Compute SHA-1(m) and convert this bit string to an integer e.
	mt_handle_t sha_handle;
	MT_CE_SHA_CTRL_S ctrl;
	ctrl.sha_para.algo_mode = xHashType;
	ret = drv_ce_sha_init(&sha_handle, &ctrl);
	
	if (CE_SUCCESS == ret)
	{
		ret = drv_ce_sha_update(sha_handle, (u8 *)pxMessage, xMessageSize);
	}
	else
	{
		return ret;
	}
	
	if (CE_SUCCESS == ret)
	{
		ret = drv_ce_sha_final(sha_handle, e);
	}
	else
	{
		return ret;
	}

	do
	{
		if (loop_s_cnt > 3)
		{
			return CE_ECDSA_SIGN_LOOP_S_FAILED;
		}
		
		do
		{
			if (loop_r_cnt > 3)
			{
				return CE_ECDSA_SIGN_LOOP_R_FAILED;
			}
			// 2. Compute kG = (x1,y1)
			if (CE_SUCCESS == ret)
			{
				ret = drv_ce_EC_POINT_mul(xParams, &kPoint, k, NULL, NULL);
			}
			else
			{
				return ret;
			}
			// 3. Compute r = x1 mod n. if r=0 then go to step 1.
			if (CE_SUCCESS == ret)
			{
				ret = drv_ce_BN_mod(r, x, n, len);
			}
			else
			{
				return ret;
			}

			loop_r_cnt++;
		}while(ecdsa_check_zero(r,len));

		// 4. Compute k(-1) mod n.
		if (CE_SUCCESS == ret)
		{
			ret = drv_ce_BN_mod_inverse(kinv, k, n, len);
		}
		else
		{
			return ret;
		}
		// 6. Compute s=k(-1)(e+dr) mod n. If s=0 then go to step 1.
		if (CE_SUCCESS == ret)
		{
			ret = drv_ce_BN_mod_mul(dr, d, r, n, len);
		}
		else
		{
			return ret;
		}
		
		if (CE_SUCCESS == ret)
		{
			ret = drv_ce_BN_mod_add(e_dr, e, dr, n, len);
		}
		else
		{
			return ret;
		}
		
		if (CE_SUCCESS == ret)
		{
			ret = drv_ce_BN_mod_mul(s, e_dr, kinv, n, len);
		}
		else
		{
			return ret;
		}

		loop_s_cnt++;
	}while(ecdsa_check_zero(s,len));

	return CE_SUCCESS;
}

RET_CODE drv_SecEcdsaVerify(MT_CE_EC_PARAMS_S xParams, MT_CE_SHA_ALGO_MODE_E xHashType, const u8 *pxPubKeyX, const u8 *pxPubKeyY, const u8 *pxMessage, u32 xMessageSize, const u8 *pxSigR, const u8 *pxSigS)
{
	RET_CODE ret;
	
	u8 *r = (u8 *)pxSigR;
	u8 *s = (u8 *)pxSigS;
	u8 *n = xParams.n;

	u32 len = xParams.keySize;
	u8 w[len];
	u8 u1[len];
	u8 u2[len];
	u8 u1X[len];
	u8 u1Y[len];
	u8 u2X[len];
	u8 u2Y[len];
	u8 x[len];
	u8 y[len];
	u8 v[len];
	u8 e[len];
	
	MT_CE_EC_POINT_S u1Gpoint;
	u1Gpoint.X = u1X;
	u1Gpoint.Y = u1Y;
	MT_CE_EC_POINT_S u2Qpoint;
	u2Qpoint.X = u2X;
	u2Qpoint.Y = u2Y;
	MT_CE_EC_POINT_S Qpoint;
	Qpoint.X = (u8 *)pxPubKeyX;
	Qpoint.Y = (u8 *)pxPubKeyY;
	MT_CE_EC_POINT_S Xpoint;
	Xpoint.X = x;
	Xpoint.Y = y;


	// 1.Verify that r and s are integers in the interval [1, n-1].
	if (ecdsa_check_interval(xParams, r))
	{
		return CE_ECDSA_VERIFY_CHECK_R_FAILED;
	}
	if (ecdsa_check_interval(xParams, s))
	{
		return CE_ECDSA_VERIFY_CHECK_S_FAILED;
	}


	// 2. Compute SHA-1(m) and convert this bit string to an integer e.
	mt_handle_t ce_handle;
	MT_CE_SHA_CTRL_S ctrl;
	ctrl.sha_para.algo_mode = xHashType;
	ret = drv_ce_sha_init(&ce_handle, &ctrl);
	
	if (CE_SUCCESS == ret)
	{
		ret = drv_ce_sha_update(ce_handle, (u8 *)pxMessage, xMessageSize);
	}
	else
	{
		return ret;
	}
	
	if (CE_SUCCESS == ret)
	{
		ret = drv_ce_sha_final(ce_handle, e);
	}
	else
	{
		return ret;
	}
	

	// 3. Compute w=s(-1) mod n.
	if (CE_SUCCESS == ret)
	{
		ret = drv_ce_BN_mod_inverse(w, s, n, len);
	}
	else
	{
		return ret;
	}

	// 4. Compute u1=ew mod n and u2 = rw mod n.
	if (CE_SUCCESS == ret)
	{
		ret = drv_ce_BN_mod_mul(u1, e, w, n, len);
	}
	else
	{
		return ret;
	}
	
	if (CE_SUCCESS == ret)
	{
		ret = drv_ce_BN_mod_mul(u2, r, w, n, len);
	}
	else
	{
		return ret;
	}


	// 5. Compute X =u1G + u2Q.
	if (CE_SUCCESS == ret)
	{
		ret = drv_ce_EC_POINT_mul(xParams, &u1Gpoint, u1, NULL, NULL);
	}
	else
	{
		return ret;
	}
	
	if (CE_SUCCESS == ret)
	{
		ret = drv_ce_EC_POINT_mul(xParams, &u2Qpoint, NULL, &Qpoint, u2);
	}
	else
	{
		return ret;
	}

	if (CE_SUCCESS == ret)
	{
		ret = drv_ce_EC_POINT_add(xParams, &Xpoint, &u1Gpoint, &u2Qpoint);
	}
	else
	{
		return ret;
	}

	// 6. If X = O, then reject the signature. Otherwise, compute v = x mod n.
	if (CE_SUCCESS == ret)
	{
		ret = drv_ce_BN_mod(v, x, n, len);
	}
	else
	{
		return ret;
	}

	// 7. Accept the signature if and only if v=r.
	if(0 == memcmp(r, v, len) )
	{
		mtos_printk("manual verify success\n");
		return CE_ECDSA_VERIFY_OK;
	}
	else
	{
		mtos_printk("manual verify failed\n");
		return CE_ECDSA_VERIFY_FAILED;
	}
}
#endif
