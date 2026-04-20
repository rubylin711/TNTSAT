/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "drv_ce_if.h"
#include "mt_mpi_ce.h"
#include "hw_m2m_if.h"
#include "m2m_cipher.h"
#include "m2m_ts.h"
#include "m2m_hash.h"
#include "m2m_mgr.h"
#include "m2m_bgc.h"
#include <linux/uaccess.h>

#define M2M_ROUNDUP(x, y)           (((x) + (y) - 1) & ~((y) - 1))
#define M2M_ROUNDDOWN(x, y)         ((x) & ~((y) - 1))

enum CIPHER_MODE {
    CIPHER_MODE_UNKNOWN = 0,
    CIPHER_MODE_SYNC,
    CIPHER_MODE_ASYNC,
};

struct cipher_req_t {
    mt_u32 channel;
    mt_u32 mode;
    m2m_cipher_req_t cipher;
};

struct hash_ctx_t {
    void *p_sha_attr;
    m2m_hash_ctx_t hash;
};

static mt_u32 m2m_get_channel(mt_u32 channel)
{
    /* REE uses channel 0 always. */
    return HW_M2M_CH0;
}

mt_s32 drv_ce_ades_create(mt_session *p_session, mt_u8 ch, void *p_priv)
{
    struct cipher_req_t *cipher;

    cipher = kmalloc(sizeof(*cipher), GFP_KERNEL);
    if (!cipher)
	return CE_GET_HANDLE_FAILED;

    memset(cipher, 0, sizeof(*cipher));

    cipher->channel = ch;
    *p_session = (mt_session)cipher;
    return CE_SUCCESS;
}

mt_s32 drv_ce_ades_destroy(mt_session session)
{
    struct cipher_req_t *cipher = (struct cipher_req_t *)session;
    if (!session)
	return CE_INVALID_HANDLE;

    m2m_cmd1_cipher_deinit(&cipher->cipher);

    kfree((void *)session);
    return CE_SUCCESS;
}

mt_s32 drv_ce_ades_config(mt_session session, MT_CE_ADES_CTRL_S *p_ctrl)
{
    struct cipher_req_t *cipher = (struct cipher_req_t *)session;
    mt_u32 algo;
    mt_u32 mode;
    mt_u32 op;
    mt_u32 profile;
    mt_u32 channel;

    if (!session)
	return CE_INVALID_HANDLE;

    if (!p_ctrl)
	return CE_BAD_PARAMETERS;

    switch (p_ctrl->ades_para.algo_mode) {
    case MT_CE_ADES_ALGO_MODE_DES:
	algo = HW_M2M_CIPHER_ALGO_DES;
	break;
    case MT_CE_ADES_ALGO_MODE_TDES_ABA:
	algo = HW_M2M_CIPHER_ALGO_TDES;
	break;
    case MT_CE_ADES_ALGO_MODE_AES128:
	algo = HW_M2M_CIPHER_ALGO_AES128;
	break;
    case MT_CE_ADES_ALGO_MODE_AES192:
	algo = HW_M2M_CIPHER_ALGO_AES192;
	break;
    case MT_CE_ADES_ALGO_MODE_AES256:
	algo = HW_M2M_CIPHER_ALGO_AES256;
	break;
    default:
	return CE_ADES_ALGO_MODE_ERROR;
	break;
    }

    switch (p_ctrl->ades_para.work_mode) {
    case MT_CE_ADES_WORK_MODE_ECB:
	mode = HW_M2M_CIPHER_MODE_ECB_CLR;
	break;
    case MT_CE_ADES_WORK_MODE_CBC:
	mode = HW_M2M_CIPHER_MODE_CBC_CLR;
	break;
    case MT_CE_AES_WORK_MODE_CTR:
	mode = HW_M2M_CIPHER_MODE_CTR;
	break;
    case MT_CE_ADES_WORK_MODE_CBCDVS042:
	mode = HW_M2M_CIPHER_MODE_CBC_SCTE52;
	break;
    case MT_CE_ADES_WORK_MODE_CBCCTS:
	mode = HW_M2M_CIPHER_MODE_CBC_CTS_CS1;
	break;
    default:
	return CE_ADES_ALGO_MODE_ERROR;
	break;
    }

    if (p_ctrl->operation == MT_CE_ADES_OPERATION_ENCRYPT)
        op = HW_M2M_CIPHER_OP_ENCRYPT;
    else
        op = HW_M2M_CIPHER_OP_DECRYPT;

    profile = HW_M2M_CIPHER_PROFILE(algo, mode, op);

    channel = m2m_get_channel(cipher->channel);

    if (m2m_cmd1_cipher_init(&cipher->cipher, profile, channel,
		p_ctrl->ades_para.key_slot, 0, 0, 0, 0) < 0)
	return CE_ADES_OTHER_ERROR;

    return CE_SUCCESS;
}

mt_s32 drv_ce_ades_process(mt_session session, mt_u8 is_phy_addr, mt_u8 *p_src_addr, mt_u8 *p_dst_addr, mt_u32 length)
{
    struct cipher_req_t *cipher = (struct cipher_req_t *)session;

    if (!session)
	return CE_INVALID_HANDLE;

    if (!is_phy_addr)
	return CE_NOT_SUPPORT;

    if (cipher->mode == CIPHER_MODE_ASYNC)
	return CE_NOT_SUPPORT;

    if (m2m_cmd1_cipher_update(&cipher->cipher, p_dst_addr, p_src_addr, 0, 0, length) < 0)
	return CE_ADES_OTHER_ERROR;

    cipher->mode = CIPHER_MODE_SYNC;
    return CE_SUCCESS;
}

mt_s32 drv_ce_ades_async_request(mt_session session, mt_u8 is_phy_addr, const mt_u8 *src, mt_u8 *dst, mt_u32 count, mt_u32 clear_len, mt_u32 prot_len)
{
    struct cipher_req_t *cipher = (struct cipher_req_t *)session;
    mt_u32 i;

    if (!session)
	return CE_INVALID_HANDLE;

    if (count == 0)
	return CE_BAD_PARAMETERS;

    if (!is_phy_addr)
	return CE_NOT_SUPPORT;

    if (cipher->mode == CIPHER_MODE_SYNC)
	return CE_NOT_SUPPORT;

    for (i = 0; i < count; i++)
    {
	if (m2m_cmd1_cipher_request(&cipher->cipher, dst, src, clear_len, 0, prot_len) < 0)
	    return CE_ADES_OTHER_ERROR;

	dst += clear_len + prot_len;
	src += clear_len + prot_len;
    }

    cipher->mode = CIPHER_MODE_ASYNC;
    return CE_SUCCESS;
}

mt_s32 drv_ce_ades_async_start(mt_session session)
{
    struct cipher_req_t *cipher = (struct cipher_req_t *)session;

    if (!session)
	return CE_INVALID_HANDLE;

    if (cipher->mode == CIPHER_MODE_SYNC)
	return CE_NOT_SUPPORT;

    if (m2m_cmd1_cipher_start_operation(&cipher->cipher) < 0)
	return CE_ADES_OTHER_ERROR;

    return CE_SUCCESS;
}

mt_s32 drv_ce_ades_async_wait(mt_session session)
{
    struct cipher_req_t *cipher = (struct cipher_req_t *)session;

    if (!session)
	return CE_INVALID_HANDLE;

    if (cipher->mode == CIPHER_MODE_SYNC)
	return CE_NOT_SUPPORT;

    if (m2m_cmd1_cipher_wait_complete(&cipher->cipher) < 0)
	return CE_ADES_OTHER_ERROR;

    return CE_SUCCESS;
}

mt_s32 drv_ce_ades_get_infor(mt_session session, MT_CE_ADES_CTRL_S *p_ctrl)
{
    return CE_SUCCESS;
}

struct drv_ts_ctx
{
    mt_u32 channel;
    m2m_ts_ctx_t ts;
    mt_u32 scr_mode;
    mt_u32 pid_num;
    mt_u16 pids[8];
};

mt_s32 drv_ce_ts_create(mt_session *p_session, mt_u8 ch, void *p_priv)
{
    struct drv_ts_ctx *ctx;

    ctx = kmalloc(sizeof(*ctx), GFP_KERNEL);
    if (!ctx)
	return CE_GET_HANDLE_FAILED;

    memset(ctx, 0, sizeof(*ctx));

    ctx->channel = ch;
    *p_session = (mt_session)ctx;
    return CE_SUCCESS;
}

mt_s32 drv_ce_ts_destroy(mt_session session)
{
    if (!session)
	return CE_INVALID_HANDLE;

    kfree((void *)session);
    return CE_SUCCESS;
}

mt_s32 drv_ce_ts_config(mt_session session, MT_CE_TS_CTRL_S *p_ctrl)
{
    struct drv_ts_ctx *ctx = (struct drv_ts_ctx *)session;
    mt_u32 algo;
    mt_u32 mode;
    mt_u32 op;
    mt_u32 profile;
    mt_u32 channel;
    mt_u32 i;

    if (!session)
	return CE_INVALID_HANDLE;

    if (!p_ctrl)
	return CE_BAD_PARAMETERS;

    switch (p_ctrl->ts_para.algo_mode) {
    case MT_CE_TS_ALGO_MODE_DES:
	algo = HW_M2M_CIPHER_ALGO_DES;
	break;
    case MT_CE_TS_ALGO_MODE_TDES_ABA:
	algo = HW_M2M_CIPHER_ALGO_TDES;
	break;
    case MT_CE_TS_ALGO_MODE_AES128:
	algo = HW_M2M_CIPHER_ALGO_AES128;
	break;
    default:
	return CE_TS_ALGO_MODE_ERROR;
	break;
    }

    switch (p_ctrl->ts_para.work_mode) {
    case MT_CE_TS_WORK_MODE_ADES_ECB:
	if (p_ctrl->ts_para.ts_short_mode == MT_CE_TS_SHORT_TAIL)
	    mode = HW_M2M_CIPHER_MODE_ECB_CLR;
	else
	    mode = HW_M2M_CIPHER_MODE_ECB_LCLR;
	break;
    case MT_CE_TS_WORK_MODE_ADES_CBC:
	if (p_ctrl->ts_para.ts_short_mode == MT_CE_TS_SHORT_TAIL)
	    mode = HW_M2M_CIPHER_MODE_CBC_CLR;
	else
	    mode = HW_M2M_CIPHER_MODE_CBC_LCLR;
	break;
    case MT_CE_TS_WORK_MODE_AES_CTR:
	mode = HW_M2M_CIPHER_MODE_CTR;
	break;
    case MT_CE_TS_WORK_MODE_ADES_CBCDVS042:
	mode = HW_M2M_CIPHER_MODE_CBC_SCTE52;
	break;
    case MT_CE_TS_WORK_MODE_ADES_CBCCTS:
	mode = HW_M2M_CIPHER_MODE_CBC_CTS_CS1;
	break;
    case MT_CE_TS_WORK_MODE_ADES_ECBCTS:
	mode = HW_M2M_CIPHER_MODE_ECB_CTS;
	break;
    default:
	return CE_TS_ALGO_MODE_ERROR;
	break;
    }

    if (p_ctrl->operation == MT_CE_TS_OPERATION_SCRAMBLE) {
        op = HW_M2M_CIPHER_OP_ENCRYPT;
	if (p_ctrl->ts_para.ts_enc_ksel == MT_CE_TS_ENC_ODD_KEY)
	    ctx->scr_mode = M2M_TS_SCR_FORCE_ODD_KEY;
	else
	    ctx->scr_mode = M2M_TS_SCR_FORCE_EVEN_KEY;
    }
    else {
        op = HW_M2M_CIPHER_OP_DECRYPT;
	if (p_ctrl->ts_para.ts_dec_ind == MT_CE_TS_DEC_IND_CLEAR)
	    ctx->scr_mode = M2M_TS_SCR_FORCE_UNSCRAMBLED;
	else
	    ctx->scr_mode = M2M_TS_SCR_NO_CHANGE;
    }

    profile = HW_M2M_CIPHER_PROFILE(algo, mode, op);

    channel = m2m_get_channel(ctx->channel);

    i = 0;
    if (p_ctrl->ts_para.ts_pid0_filt_en == MT_CE_TS_PID_FILT_ENABLE)
	ctx->pids[i++] = p_ctrl->ts_para.ts_pid0_filt_num;
    if (p_ctrl->ts_para.ts_pid1_filt_en == MT_CE_TS_PID_FILT_ENABLE)
	ctx->pids[i++] = p_ctrl->ts_para.ts_pid1_filt_num;
    if (p_ctrl->ts_para.ts_pid2_filt_en == MT_CE_TS_PID_FILT_ENABLE)
	ctx->pids[i++] = p_ctrl->ts_para.ts_pid2_filt_num;
    if (p_ctrl->ts_para.ts_pid3_filt_en == MT_CE_TS_PID_FILT_ENABLE)
	ctx->pids[i++] = p_ctrl->ts_para.ts_pid3_filt_num;
    if (p_ctrl->ts_para.ts_pid4_filt_en == MT_CE_TS_PID_FILT_ENABLE)
	ctx->pids[i++] = p_ctrl->ts_para.ts_pid4_filt_num;
    if (p_ctrl->ts_para.ts_pid5_filt_en == MT_CE_TS_PID_FILT_ENABLE)
	ctx->pids[i++] = p_ctrl->ts_para.ts_pid5_filt_num;
    if (p_ctrl->ts_para.ts_pid6_filt_en == MT_CE_TS_PID_FILT_ENABLE)
	ctx->pids[i++] = p_ctrl->ts_para.ts_pid6_filt_num;
    if (p_ctrl->ts_para.ts_pid7_filt_en == MT_CE_TS_PID_FILT_ENABLE)
	ctx->pids[i++] = p_ctrl->ts_para.ts_pid7_filt_num;

    ctx->pid_num = i;

    if (p_ctrl->ts_para.ts_force_enc_en == MT_CE_TS_FORCE_ENC_ENABLE)
	m2m_enable_ts_mode_force_enc(channel);
    else
	m2m_disable_ts_mode_force_enc(channel);

    if (m2m_ts_init(&ctx->ts, profile, channel, p_ctrl->ts_para.even_key_slot,
		p_ctrl->ts_para.odd_key_slot, 0, 0) < 0)
	return CE_TS_OTHER_ERROR;

    return CE_SUCCESS;
}

mt_s32 drv_ce_ts_process(mt_session session, mt_u8 is_phy_addr, mt_u8 *p_src_addr, mt_u8 *p_dst_addr, mt_u32 length)
{
    struct drv_ts_ctx *ctx = (struct drv_ts_ctx *)session;

    (void)is_phy_addr; //not used

    if (!session)
	return CE_INVALID_HANDLE;

    if (m2m_ts_process(&ctx->ts, ctx->pid_num, ctx->pids, ctx->scr_mode, p_dst_addr, p_src_addr, length) < 0)
	return CE_ADES_OTHER_ERROR;

    return CE_SUCCESS;
}

mt_s32 drv_ce_ts_get_infor(mt_session session, MT_CE_TS_CTRL_S *p_ctrl)
{
    return CE_SUCCESS;
}

mt_s32 drv_ce_sha_create(mt_session *p_session, void *p_priv)
{
    struct hash_ctx_t *hash;

    hash = kmalloc(sizeof(*hash), GFP_KERNEL);
    if (!hash)
	return CE_GET_HANDLE_FAILED;

    memset(hash, 0, sizeof(*hash));
    *p_session = (mt_session)hash;
    return CE_SUCCESS;
}

mt_s32 drv_ce_sha_destroy(mt_session session)
{
    struct hash_ctx_t *hash = (struct hash_ctx_t *)session;

    if (!session)
	return CE_INVALID_HANDLE;

    if (hash->p_sha_attr) {
        kfree(hash->p_sha_attr);
        hash->p_sha_attr = NULL;
    }

    kfree((void *)session);
    return CE_SUCCESS;
}

mt_s32 drv_ce_sha_init(mt_session session, MT_CE_SHA_CTRL_S *p_ctrl)
{
    struct hash_ctx_t *hash = (struct hash_ctx_t *)session;
    mt_u32 profile;
    mt_u32 channel;

    if (!session)
	return CE_INVALID_HANDLE;

    if (!p_ctrl)
	return CE_BAD_PARAMETERS;

    switch (p_ctrl->sha_para.algo_mode) {
    case MT_CE_HASH_ALGO_MODE_SHA1:
	profile = SHA1;
	break;
    /*
    case MT_CE_HASH_ALGO_MODE_SHA224:
	profile = SHA2_224;
	break;
    */
    case MT_CE_HASH_ALGO_MODE_SHA256:
	profile = SHA2_256;
	break;
    /*
    case MT_CE_HASH_ALGO_MODE_SHA384:
	profile = SHA2_384;
	break;
    case MT_CE_HASH_ALGO_MODE_SHA512:
	profile = SHA2_512;
	break;
    */
    case MT_CE_HASH_ALGO_MODE_HMAC_SHA256:
	profile = HMAC_SHA2_256;
	break;
    default:
	return CE_BAD_PARAMETERS;
	break;
    }

    hash->p_sha_attr = (void *)kmalloc(sizeof(MT_CE_SHA_PARA_S), GFP_KERNEL);
    memcpy((MT_CE_SHA_PARA_S *)hash->p_sha_attr, &p_ctrl->sha_para, sizeof(MT_CE_SHA_PARA_S));

    channel = m2m_get_channel(p_ctrl->sha_para.chan_num);

    if (m2m_cmd0_hash_init(&hash->hash, profile, channel) < 0)
	return CE_SHA_INIT_FAILED;

    return CE_SUCCESS;
}

mt_s32 drv_ce_sha_get_attr(mt_session session, MT_CE_SHA_CTRL_S *p_attr)
{
    struct hash_ctx_t *hash = (struct hash_ctx_t *)session;
    MT_CE_SHA_PARA_S *p_sha_para = (MT_CE_SHA_PARA_S *)hash->p_sha_attr;

    if (!session)
	return CE_INVALID_HANDLE;

    if (p_attr == NULL)
	return CE_BAD_PARAMETERS;

    if (p_sha_para) {
	memcpy(&p_attr->sha_para, p_sha_para, sizeof(MT_CE_SHA_PARA_S)); 
    }

    return CE_SUCCESS;
}

mt_s32 drv_ce_sha_update(mt_session session, mt_u8 is_phy_addr, mt_u8 *p_msg, mt_u32 length)
{
    struct hash_ctx_t *hash = (struct hash_ctx_t *)session;

    if (!session)
	return CE_INVALID_HANDLE;

    if (!is_phy_addr)
	return CE_NOT_SUPPORT;

    if (m2m_cmd0_hash_update(&hash->hash, p_msg, length) < 0)
	return CE_SHA_OTHER_ERROR;

    return CE_SUCCESS;
}

mt_s32 drv_ce_sha_final(mt_session session, u8 *p_dgst)
{
    struct hash_ctx_t *hash = (struct hash_ctx_t *)session;

    if (!session)
	return CE_INVALID_HANDLE;

    if (!p_dgst)
	return CE_BAD_PARAMETERS;

    if (m2m_cmd0_hash_final(&hash->hash, p_dgst) < 0)
	return CE_SHA_OTHER_ERROR;

    return CE_SUCCESS;
}

mt_s32 drv_ce_sha_update_start(mt_session session, u8 *p_msg, u32 length)
{
    return CE_SUCCESS;
}

mt_s32 drv_ce_sha_update_polling(mt_session session, u32 time_out)
{
    return CE_SUCCESS;
}

mt_s32 drv_ce_sha_update_stop(mt_session session)
{
    return CE_SUCCESS;
}

mt_s32 drv_ce_bgc_request(mt_session *p_session, MT_CE_BGC_SEM_REQ_S req_timeout)
{
    mt_s32 req_slot = 0;

    if (p_session == NULL) {
        return CE_INVALID_HANDLE;
    }
    
    if (m2m_bgc_sem_wait(M2M_BGC_SEM_CPU_ID_REECPU, req_timeout)) {
        return CE_GET_HANDLE_FAILED;
    }

    req_slot = m2m_bgc_slot_request();
    if (req_slot <= CE_SUCCESS) {
        printk("%s, there are no available slot.\n", __FUNCTION__);
        return CE_GET_HANDLE_FAILED; 
    }

    *p_session = req_slot;
  
    return CE_SUCCESS;
}

mt_s32 drv_ce_bgc_setup(mt_session session, const mt_u8 *bgc_addr, mt_u32 size, MT_CE_BGC_DELAY delay, mt_u8 *golden_hash, MT_CE_BGC_LOCK_S lock)
{
    extern mt_u8 _text[], _etext[];
    mt_s32 ret = CE_GET_HANDLE_FAILED;
    mt_session hash_session;
    MT_CE_SHA_CTRL_S sha_ctrl;
    mt_u32 bgc_slot = session, bgc_size = size;
    mt_u8 *bgc_start_addr = (mt_u8 *)bgc_addr;
    mt_u8 digest[32];

    if (!m2m_bgc_sem_check(M2M_BGC_SEM_CPU_ID_REECPU)) {
        return CE_GET_HANDLE_FAILED;
    }

    /* bgc_size == bgc_addr == 0, setup kernel text+rodata-section bgc, hw support bgc_addr in bytes unit */
    if ((bgc_size == 0) && (bgc_addr == NULL)) {
        bgc_start_addr = (mt_u8 *)_text;
        bgc_size = (mt_u32)_etext - (mt_u32)_text;
        bgc_start_addr = (mt_u8 *)virt_to_phys((const volatile void *)bgc_start_addr);
    } else if ((bgc_size == 0) || (bgc_addr == NULL)) {
        return CE_GET_HANDLE_FAILED;
    }

    /* golden_hash = 0, driver calculate golden hash by itself */
    if (golden_hash == NULL) {        
        sha_ctrl.sha_para.algo_mode = MT_CE_HASH_ALGO_MODE_SHA256;
        if (drv_ce_sha_create(&hash_session, NULL)) {
            goto bgc_setup_exit;
        }
        if (drv_ce_sha_init(hash_session, &sha_ctrl)) {
            goto bgc_setup_exit;
        }        
        if (drv_ce_sha_update(hash_session, 1, bgc_start_addr, bgc_size)) {
            goto bgc_setup_exit;
        }
        if (drv_ce_sha_final(hash_session, digest)) {
            goto bgc_setup_exit;
        }
    } else {
        copy_from_user(digest, (void __user *)golden_hash, sizeof(digest));
    }

    if (m2m_bgc_slot_start(bgc_slot, (const mt_u8 *)bgc_start_addr, bgc_size, digest, delay)) {
        goto bgc_setup_exit;
    }
    if (lock) {
        if (m2m_bgc_slot_lock(bgc_slot)) {
            m2m_bgc_slot_stop(bgc_slot);
            goto bgc_setup_exit;
        }        
    }
    ret = CE_SUCCESS;
    
bgc_setup_exit:
    if (golden_hash == NULL) {
        drv_ce_sha_destroy(hash_session);
    }
    m2m_bgc_sem_post();
    return ret;
}

mt_s32 drv_ce_bgc_release(mt_session session, MT_CE_BGC_SEM_REQ_S req_timeout)
{
    mt_s32 ret = CE_GET_HANDLE_FAILED;
    
    if (m2m_bgc_sem_wait(M2M_BGC_SEM_CPU_ID_REECPU, req_timeout)) {
        return CE_GET_HANDLE_FAILED;
    }

    ret = m2m_bgc_slot_stop(session);

    m2m_bgc_sem_post();

    return ret;
}

void drv_ce_ades_init(void)
{
	/* TODO */
}

void drv_ce_ades_deinit(void)
{
	/* TODO */
}
