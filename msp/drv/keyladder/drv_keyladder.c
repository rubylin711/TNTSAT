/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/semaphore.h>
#include <linux/delay.h>
#include "drv_keyladder.h"
#ifdef CONFIG_MT_CHIP_SYMPHONY6
#include "sym6/hw_keyladder_bare_api.h"
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
#include "sym4/hw_keyladder_bare_api.h"
#else
#include "sym2/hw_keyladder_bare_api.h"
#endif

typedef struct _kl_drv_priv_s
{
    struct semaphore sem;
    void *handle;
} kl_drv_priv_s;

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
static kl_drv_priv_s   g_kl_t;
#else
static kl_drv_priv_s   g_kl_cw;
static kl_drv_priv_s   g_kl_pvr;
#endif

void kl_delay_function(unsigned long ms)
{
    msleep(ms);
}

mt_s32 DRV_KEYLADDER_Init(mt_void)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	sema_init(&g_kl_t.sem, 1);
	bare_kl_init(&g_kl_t.handle);
#else
	sema_init(&g_kl_cw.sem, 1);
	bare_kl_init(&g_kl_cw.handle, KL_TYPE_0);
	sema_init(&g_kl_pvr.sem, 1);
	bare_kl_init(&g_kl_pvr.handle, KL_TYPE_1);
#endif
	return 0;
}

mt_s32 DRV_KEYLADDER_DeInit(mt_void)
{
    return 0;
}

mt_s32 DRV_KEYLADDER_Open(KEYLADDER_TYPE_E type, mt_handle *p_handle)
{
	mt_s32 ret = -1;

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	*p_handle = (mt_handle)&g_kl_t;
	ret = 0;
#else
    if (type == KL_TYPE_0) {
        *p_handle = (mt_handle)&g_kl_cw;
        ret = 0;
    } else if (type == KL_TYPE_1) {
        *p_handle = (mt_handle)&g_kl_pvr;
        ret = 0;
    } else {
        *p_handle = 0;
    }
#endif
    return ret;
}

mt_s32 DRV_KEYLADDER_Close(mt_handle handle)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;

    return bare_kl_deinit(p_priv->handle);
#else
    return 0;
#endif
}

mt_s32 DRV_KEYLADDER_ReqSem(mt_handle handle)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;

	if (bare_kl_sem_request(p_priv->handle, KL_SEM_CPU_ID_APCPU)) {
		return -1;
	}
#endif
    return 0;
}

mt_s32 DRV_KEYLADDER_RlsSem(mt_handle handle)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;

	if (bare_kl_sem_release(p_priv->handle)) {
		return -1;
	}
#endif
    return 0;
}

mt_s32 DRV_KEYLADDER_Lock(mt_handle handle)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    return down_interruptible(&p_priv->sem);
}

mt_s32 DRV_KEYLADDER_Unlock(mt_handle handle)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    up(&p_priv->sem);
    return 0;
}

mt_s32 DRV_KEYLADDER_SetSignature(mt_handle handle, mt_u8 *p_signature)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    return bare_kl_set_signature(p_priv->handle, p_signature);
}

mt_s32 DRV_KEYLADDER_SelectRootkey(mt_handle handle, KL_SCK_SOURCE_E rootkey_source)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    return bare_kl_select_rootkey(p_priv->handle, rootkey_source);
}

mt_s32 DRV_KEYLADDER_LinkAES(mt_handle handle, mt_u8 *p_input, KL_MOVE_ENC_E enc_type)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    return bare_kl_link_aes(p_priv->handle, p_input, enc_type);
#else
	return bare_kl_link_aes(p_priv->handle, p_input);
#endif
}

mt_s32 DRV_KEYLADDER_LinkTDES(mt_handle handle, mt_u8 *p_input, KL_MOVE_ENC_E enc_type)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    return bare_kl_link_tdes(p_priv->handle, p_input, enc_type);
#else
	return bare_kl_link_tdes(p_priv->handle, p_input);
#endif
}

mt_s32 DRV_KEYLADDER_LinkXOR(mt_handle handle, mt_u8 *p_input)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    return bare_kl_link_xor(p_priv->handle, p_input);
}

mt_s32 DRV_KEYLADDER_LinkHash(mt_handle handle, mt_u8 *p_input, KL_HASH_POSITION_E input_data_pos, KL_HASH_POSITION_E output_key_pos)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    return bare_kl_link_hash(p_priv->handle, p_input, input_data_pos, output_key_pos);
}

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 DRV_KEYLADDER_LinkSeedv(mt_handle handle, mt_u8 *p_input, KL_HARDWIRED_SOURCE_E mask_key, KL_STANDARD_PROFILE_E profile)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    return bare_kl_link_seedv(p_priv->handle, p_input, mask_key, profile);
}
#else
mt_s32 DRV_KEYLADDER_LinkSeedv(mt_handle handle, KL_HARDWIRED_SOURCE_E mask_key, KL_STANDARD_PROFILE_E profile)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    return bare_kl_link_seedv(p_priv->handle, mask_key, profile);
}
#endif

mt_s32 DRV_KEYLADDER_StoreKey(mt_handle handle, KL_STORE_DST_E store_dst)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    return bare_kl_store_key(p_priv->handle, store_dst);
}

mt_s32 DRV_KEYLADDER_ExportKey(mt_handle handle, KL_EXPORT_DST_E export_dst, mt_u32 slot_id)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    return bare_kl_export_key(p_priv->handle, export_dst, slot_id);
}

mt_s32 DRV_KEYLADDER_InputData(mt_handle handle, KL_INPUT_POSITION_E postion, mt_u8 *p_input)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    return bare_kl_input_data(p_priv->handle, postion, p_input);
}

mt_s32 DRV_KEYLADDER_MoveCmd(mt_handle handle, KL_MOVE_SRC_E move_src, KL_MOVE_DST_E move_dst, KL_MOVE_TRIGGER_E trigger, KL_MOVE_ENC_E enc_type)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    return bare_kl_move_cmd(p_priv->handle, move_src, move_dst, trigger, enc_type);
#else
	return bare_kl_move_cmd(p_priv->handle, move_src, move_dst, trigger);
#endif
}

mt_s32 DRV_KEYLADDER_StoreCmd(mt_handle handle, KL_STORE_SRC_E store_src, KL_STORE_DST_E store_dst, KL_STORE_LOCK_E lock)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    return bare_kl_store_cmd(p_priv->handle, store_src, store_dst, lock);
}

mt_s32 DRV_KEYLADDER_ExportCmd(mt_handle handle, KL_EXPORT_SOURCE_E key_src, KL_EXPORT_DST_E export_dst, mt_u32 slot_id)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    return bare_kl_export_cmd(p_priv->handle, key_src, export_dst, slot_id);
}

mt_s32 DRV_KEYLADDER_SetSigSrc(mt_handle handle, KL_SIG_CPU_E cpu, KL_SIG_KEYSRC_E keysrc)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    return bare_kl_set_signature_keysrc(p_priv->handle, cpu, keysrc);
}

mt_s32 DRV_KEYLADDER_LockSigSrc(mt_handle handle, KL_SIG_CPU_E cpu, unsigned char lock)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    return bare_kl_set_signature_lock_keysrc(p_priv->handle, cpu, lock);
}

mt_s32 DRV_KEYLADDER_WaitComplete(mt_handle handle, mt_u32 *p_error)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    return bare_kl_wait_complete(p_priv->handle, (unsigned long *)p_error);
}

mt_s32 DRV_KEYLADDER_ReadKey(mt_handle handle, mt_u8 *p_key_buffer)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;
    return bare_kl_read_key(p_priv->handle, p_key_buffer);
}

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 DRV_KEYLADDER_ExtrTDC(mt_handle handle, mt_u8 *p_input)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;

    return bare_kl_store_tdc(p_priv->handle, p_input);
}

mt_s32 DRV_KEYLADDER_RunTDC(mt_handle handle, mt_u8 *p_input, KL_MOVE_ALGO_E algo_type, KL_MOVE_ENC_E enc_type)
{
    kl_drv_priv_s *p_priv = (kl_drv_priv_s *)handle;

    return bare_kl_execute_tdc(p_priv->handle, p_input, algo_type, enc_type);
}

mt_s32 DRV_KEYLADDER_Additions(mt_handle handle, KL_ADDITIONS_E addt, KL_FUNC_ENABLE_E en)
{
    return bare_kl_addt_condition_op(addt, en);
}
#endif
