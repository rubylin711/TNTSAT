/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_CE_IF_H__
#define __DRV_CE_IF_H__

#include <linux/slab.h>
#include <linux/uaccess.h>
#include "mt_mpi_ce.h"
#include "mt_drv_dev.h"
#include "drv_ce_ioctl.h"

#define DRV_CE_UNUSED(x)    (void)x

//==========================AUXILIARY================================
typedef struct semaphore	os_sem_t;
typedef struct _MT_CE_AUX_S
{
  mt_session keep_ch0;
  mt_session keep_ch1;
  mt_session keep_ch2;
  mt_session keep_ch3;

  union {
      os_sem_t sem_ch0;
      struct completion sem;
  };

  os_sem_t sem_ch1;
  os_sem_t sem_ch2;
  os_sem_t sem_ch3;
  os_sem_t sem_rsa;
  os_sem_t sem_ecc;
} MT_CE_AUX_S;

typedef struct _DRV_PKA_RSA_CTRL_S
{
    void *priv;

    mt_u8 p_m[256];
    mt_u8 p_e[256];
    mt_u32 key_length;
    mt_u32 exp_length;
} DRV_PKA_RSA_CTRL_S;

void drv_ce_aux_init(MT_CE_AUX_S *p_aux);
void drv_ce_aux_deinit(MT_CE_AUX_S *p_aux);

void drv_ce_ades_init(void);
void drv_ce_ades_deinit(void);

void *drv_ce_get_handle(void);

mt_s32 drv_ce_ades_create(mt_session *p_session, mt_u8 ch, void *p_priv);
mt_s32 drv_ce_ades_destroy(mt_session session);
mt_s32 drv_ce_ades_config(mt_session session, MT_CE_ADES_CTRL_S *p_ctrl);
mt_s32 drv_ce_ades_get_infor(mt_session session, MT_CE_ADES_CTRL_S *p_ctrl);
mt_s32 drv_ce_ades_process(mt_session session, mt_u8 is_phy_addr, phys_addr_t p_src_addr, phys_addr_t p_dst_addr, mt_u32 clear_len, mt_u32 prot_len);
mt_s32 drv_ce_ades_grp_process(mt_session session, MT_CE_GRP_MODE_E grp, mt_u8 *p_iv_addr, mt_u8 *p_src_addr, mt_u8 *p_dst_addr, mt_u32 length);

mt_s32 drv_ce_ades_grp_process_start(mt_session session, MT_CE_GRP_MODE_E grp, mt_u8 *p_iv_addr, mt_u8 *p_src_addr, mt_u8 *p_dst_addr, mt_u32 length);
mt_s32 drv_ce_ades_process_polling(mt_session session, mt_u32 time_out);
mt_s32 drv_ce_ades_process_stop(mt_session session);

//Async api
mt_s32 drv_ce_ades_process_start(mt_session session, mt_u8 is_phy_addr, mt_u8 *p_src_addr, mt_u8 *p_dst_addr, mt_u32 length);
mt_s32 drv_ce_ades_process_wait(mt_session session, mt_u32 timeout);

mt_s32 drv_ce_ades_async_request(mt_session session, mt_u8 is_phy_addr, phys_addr_t src, phys_addr_t dst, mt_u32 count, mt_u32 clear_len, mt_u32 prot_len);
mt_s32 drv_ce_ades_async_start(mt_session session);
mt_s32 drv_ce_ades_async_wait(mt_session session);

mt_s32 drv_ce_ts_create(mt_session *p_session, mt_u8 ch, void *p_priv);
mt_s32 drv_ce_ts_destroy(mt_session session);
mt_s32 drv_ce_ts_config(mt_session session, MT_CE_TS_CTRL_S *p_ctrl);
mt_s32 drv_ce_ts_get_infor(mt_session session, MT_CE_TS_CTRL_S *p_ctrl);
mt_s32 drv_ce_ts_process(mt_session session, mt_u8 is_phy_addr, phys_addr_t p_src_addr, phys_addr_t p_dst_addr, mt_u32 length);

mt_s32 drv_ce_sha_create(mt_session *p_session, void *p_priv);
mt_s32 drv_ce_sha_destroy(mt_session session);
mt_s32 drv_ce_sha_init(mt_session session, MT_CE_SHA_CTRL_S *p_ctrl);
mt_s32 drv_ce_sha_update(mt_session session, mt_u8 is_phy_addr, phys_addr_t p_msg, mt_u32 length);
mt_s32 drv_ce_sha_final(mt_session session, mt_u8 *p_dgst);
mt_s32 drv_ce_sha_get_attr(mt_session session, MT_CE_SHA_CTRL_S *p_attr);

mt_s32 drv_ce_mac_create(mt_session *p_session, void *p_priv);
mt_s32 drv_ce_mac_destroy(mt_session session);
mt_s32 drv_ce_mac_init(mt_session session, MT_CE_SHA_CTRL_S *p_ctrl);
mt_s32 drv_ce_mac_update(mt_session session, mt_u8 is_phy_addr, phys_addr_t p_msg, mt_u32 length);
mt_s32 drv_ce_mac_final(mt_session session, mt_u8 *p_dgst);

mt_s32 drv_ce_rsa_create(mt_session *p_session, void *p_priv);
mt_s32 drv_ce_rsa_destroy(mt_session session);
mt_s32 drv_ce_rsa_config(mt_session session, MT_CE_RSA_CTRL_S *p_ctrl);
mt_s32 drv_ce_rsa_process(mt_session session, mt_u8 *p_src_addr, mt_u8 *p_dst_addr, mt_u32 src_length);

mt_s32 drv_ce_BN_create(mt_session *p_session, void *p_priv);
mt_s32 drv_ce_BN_destroy(mt_session session);
mt_s32 drv_ce_BN_mod_mod(mt_session session, mt_u8 *r, mt_u8 *d, mt_u8 *m, mt_u32 d_length, mt_u32 m_length);
mt_s32 drv_ce_BN_mod_mul(mt_session session, mt_u8 *r, mt_u8 *a, mt_u8 *b, mt_u8 *m, mt_u32 a_length, mt_u32 b_length, mt_u32 m_length);
mt_s32 drv_ce_BN_mod_add(mt_session session, mt_u8 *r, mt_u8 *a, mt_u8 *b, mt_u8 *m, mt_u32 a_length, mt_u32 b_length, mt_u32 m_length);
mt_s32 drv_ce_BN_mod_sub(mt_session session, mt_u8 *r, mt_u8 *a, mt_u8 *b, mt_u8 *m, mt_u32 a_length, mt_u32 b_length, mt_u32 m_length);
mt_s32 drv_ce_BN_mod_inverse(mt_session session, mt_u8 *r, mt_u8 *d, mt_u8 *m, mt_u32 d_length, mt_u32 m_length);
mt_s32 drv_ce_BN_mod_exp(mt_session session, mt_u8 *r, mt_u8 *a, mt_u8 *p, mt_u8 *m, mt_u32 a_length, mt_u32 p_length, mt_u32 m_length);

mt_s32 drv_ce_EC_POINT_create(mt_session *p_session, void *p_priv);
mt_s32 drv_ce_EC_POINT_destroy(mt_session session);
mt_s32 drv_ce_EC_POINT_mul(mt_session session, MT_CE_EC_PARAMS_S xParams, MT_CE_EC_POINT_S *r, const mt_u8 *g_scalar,  MT_CE_EC_POINT_S *p_point, const mt_u8 *p_scalar);
mt_s32 drv_ce_EC_POINT_add(mt_session session, MT_CE_EC_PARAMS_S xParams, MT_CE_EC_POINT_S *r, const MT_CE_EC_POINT_S *a, const MT_CE_EC_POINT_S *b);

#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 drv_ce_bgc_request(mt_session *p_session, MT_CE_BGC_SEM_REQ_S req_timeout);
mt_s32 drv_ce_bgc_setup(mt_session session, const mt_u8 *bgc_addr, mt_u32 size, MT_CE_BGC_DELAY delay, mt_u8 *golden_hash, MT_CE_BGC_LOCK_S lock);
mt_s32 drv_ce_bgc_release(mt_session session, MT_CE_BGC_SEM_REQ_S req_timeout);
#endif

mt_s32 drv_ce_reset(mt_u32 mode);

/* utils */
mt_u8 *drv_ce_copy_from_user_data(mt_u8 *buf, mt_u32 length);
mt_s32 drv_ce_copy_to_user_data(mt_u8 *src_buf, mt_u8 *dst_buf, mt_u32 length);

void dump(const char *tag, mt_u8 *buffer, mt_u32 len);

#endif	/*__DRV_CE_IF_H__*/
