/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "drv_keyladder_ioctl.h"
#include "mt_mpi_keyladder.h"

mt_s32 mt_mpi_kl_open(KEYLADDER_TYPE_E type, mt_handle *p_handle)
{
    int fd = -1;

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	fd = open("/dev/mt_keyladder", O_RDWR);
	if(fd >= 0)
	{
		*p_handle = (mt_handle)fd;
		return MT_SUCCESS;
	}
#else
    if(type == KL_TYPE_0)
    {
        fd = open("/dev/mt_keyladder0", O_RDWR);
    }
    else if(type == KL_TYPE_1)
    {
        fd = open("/dev/mt_keyladder1", O_RDWR);
    }
    if(fd >= 0)
    {
        *p_handle = (mt_handle)fd;
        return MT_SUCCESS;
    }
#endif
    //very important
    *p_handle = MT_INVALID_HANDLE;

    return MT_FAILURE;
}

mt_s32 mt_mpi_kl_close(mt_handle handle)
{
    return close((int)handle);
}

mt_s32 mt_mpi_kl_lock(mt_handle handle)
{
    return ioctl((int)handle, KEYLADDER_DRV_IOC_LOCK, NULL);
}

mt_s32 mt_mpi_kl_unlock(mt_handle handle)
{
    return ioctl((int)handle, KEYLADDER_DRV_IOC_UNLOCK, NULL);
}

mt_s32 mt_mpi_kl_request_sem(mt_handle handle)
{
    return ioctl((int)handle, KEYLADDER_DRV_IOC_REQSEM, NULL);
}

mt_s32 mt_mpi_kl_release_sem(mt_handle handle)
{
    return ioctl((int)handle, KEYLADDER_DRV_IOC_RLSSEM, NULL);
}

mt_s32 mt_mpi_kl_set_signature(mt_handle handle, mt_u8 *p_signature)
{
    return ioctl((int)handle, KEYLADDER_DRV_IOC_SETSIGNATURE, p_signature);
}

mt_s32 mt_mpi_kl_select_rootkey(mt_handle handle, KL_SCK_SOURCE_E rootkey_source)
{
    mt_u32 source = rootkey_source;
    return ioctl((int)handle, KEYLADDER_DRV_IOC_SELECTROOTKEY, &source);
}

mt_s32 mt_mpi_kl_link_aes(mt_handle handle, mt_u8 *p_input, KL_MOVE_ENC_E enc_type)
{
    KL_LINK_CRYPTO_S kl_link = {0};

    kl_link.enc_ptye = enc_type;
    memcpy(kl_link.input, p_input, 16);

    return ioctl((int)handle, KEYLADDER_DRV_IOC_LINKAES, &kl_link);
}

mt_s32 mt_mpi_kl_link_tdes(mt_handle handle, mt_u8 *p_input, KL_MOVE_ENC_E enc_type)
{
    KL_LINK_CRYPTO_S kl_link = {0};

    kl_link.enc_ptye = enc_type;
    memcpy(kl_link.input, p_input, 16);

    return ioctl((int)handle, KEYLADDER_DRV_IOC_LINKTDES, &kl_link);
}

mt_s32 mt_mpi_kl_export_key(mt_handle handle, KL_EXPORT_DST_E kl_dst, mt_u32 slot_id)
{
    KL_EXPORT_KEY_S kl_exp = {0};
    kl_exp.export_dst = kl_dst;
    kl_exp.slot_id = slot_id;
    return ioctl((int)handle, KEYLADDER_DRV_IOC_EXPORTKEY, &kl_exp);
}

mt_s32 mt_mpi_kl_wait_complete(mt_handle handle, mt_s32 *p_error)
{
    mt_s32 error = -1;
    if(p_error != NULL)
        return ioctl((int)handle, KEYLADDER_DRV_IOC_WAITCOMPLETE, p_error);
    return ioctl((int)handle, KEYLADDER_DRV_IOC_WAITCOMPLETE, &error);
}

mt_s32 mt_mpi_kl_read_key(mt_handle handle, mt_u8 *p_key_buffer)
{
    return ioctl((int)handle, KEYLADDER_DRV_IOC_READKEY, p_key_buffer);
}

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 mt_mpi_kl_InputData(mt_handle handle, KL_INPUT_POSITION_E postion, mt_u8 *p_input)
{
    KL_INPUT_DATA_S input_data;
    
    input_data.postion = postion;
    memcpy(input_data.input, p_input, 16);
    return ioctl((int)handle, KEYLADDER_DRV_IOC_INPUTDATA, &input_data);
}

mt_s32 mt_mpi_kl_LinkSeedv(mt_handle handle, mt_u8 *p_input, KL_HARDWIRED_SOURCE_E mask_key, KL_STANDARD_PROFILE_E profile)
{
    KL_LINK_SEEDV_S link_seedv;
    
    link_seedv.mask_key = mask_key;
    link_seedv.profile = profile;
    memcpy(link_seedv.input, p_input, 16);
    return ioctl((int)handle, KEYLADDER_DRV_IOC_LINKSEEDV, &link_seedv);
}

mt_s32 mt_mpi_kl_extr_tdc(mt_handle handle, mt_u8 *tdc)
{
    return ioctl((int)handle, KEYLADDER_DRV_IOC_EXTRTDC, tdc);
}

mt_s32 mt_mpi_kl_run_tdc(mt_handle handle, mt_u8 *p_input, KL_MOVE_ALGO_E algo_type, KL_MOVE_ENC_E enc_type)
{
    KL_LINK_CRYPTO_S kl_link = {0};

    kl_link.algo_type = algo_type;
    kl_link.enc_ptye = enc_type;
    memcpy(kl_link.input, p_input, 16);

    return ioctl((int)handle, KEYLADDER_DRV_IOC_RUNTDC, &kl_link);
}

mt_s32 mt_mpi_kl_storeKey(mt_handle handle, KL_STORE_DST_E store_dst)
{
    mt_u32 dst = store_dst;
    return ioctl((int)handle, KEYLADDER_DRV_IOC_STOREKEY, &dst);
}

mt_s32 mt_mpi_kl_extra(mt_handle handle, KL_ADDITIONS_E addt, KL_FUNC_ENABLE_E en)
{
    KL_ADDITIONS_S kl_add_condition = {0};

    kl_add_condition.addt = addt;
    kl_add_condition.en = en;
    return ioctl((int)handle, KEYLADDER_DRV_IOC_ADDITION, &kl_add_condition);
}
#endif

#if 0
mt_s32 mt_mpi_kl_LinkXOR(mt_handle handle, mt_u8 *p_input)
{
    return ioctl((int)handle, KEYLADDER_DRV_IOC_LINKXOR, p_input);
}

mt_s32 mt_mpi_kl_LinkHash(mt_handle handle, mt_u8 *p_input, KL_HASH_POSITION_E input_data_pos, KL_HASH_POSITION_E output_key_pos)
{
    KL_LINK_HASH_S link_hash;
    memcpy(link_hash.input, p_input, 16);
    link_hash.input_data_pos = input_data_pos;
    link_hash.output_key_pos = output_key_pos; 
    return ioctl((int)handle, KEYLADDER_DRV_IOC_LINKHASH, &link_hash);
}

mt_s32 mt_mpi_kl_MoveCmd(mt_handle handle, KL_MOVE_SRC_E move_src, KL_MOVE_DST_E move_dst, KL_MOVE_TRIGGER_E trigger)
{
    KL_MOVE_CMD_S move_cmd;
    move_cmd.move_src = move_src;
    move_cmd.move_dst = move_dst;
    move_cmd.trigger = trigger;
    return ioctl((int)handle, KEYLADDER_DRV_IOC_MOVECMD, &move_cmd);
}

mt_s32 mt_mpi_kl_StoreCmd(mt_handle handle, KL_STORE_SRC_E store_src, KL_STORE_DST_E store_dst, KL_STORE_LOCK_E lock)
{
    KL_SOTRE_CMD_S store_cmd;
    store_cmd.store_src = store_src; 
    store_cmd.store_dst = store_dst;
    store_cmd.lock = lock;
    return ioctl((int)handle, KEYLADDER_DRV_IOC_STORECMD, &store_cmd);
}

mt_s32 mt_mpi_kl_ExportCmd(mt_handle handle, KL_EXPORT_SOURCE_E key_src)
{
    KL_EXPORT_CMD_S export_cmd;
    export_cmd.key_src = key_src;
    export_cmd.slot_id = MT_KT_SLOT_ID_INVALID; //use keyladder internal slot id
    return ioctl((int)handle, KEYLADDER_DRV_IOC_EXPORTCMD, &export_cmd);
}
#endif
