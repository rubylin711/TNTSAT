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
#include "mt_mpi_kt.h"
#include "drv_kt_ioctl.h"
#include "mt_unf_cipher_v2.h"
#include "mt_mpi_cipher_v2.h"

mt_s32 mt_mpi_kt_open(mt_handle *p_handle)
{
    int fd = -1;

    fd = open("/dev/mt_keytable", O_RDWR);
    
    if(fd >= 0)
    {
        *p_handle = (mt_handle)fd;
        return MT_SUCCESS;
    }
    
    return MT_FAILURE;
}

mt_s32 mt_mpi_kt_close(mt_handle handle)
{
    if (handle == 0)
        return MT_FAILURE;

    return close((int)handle);
}

mt_s32 mt_mpi_kt_slot_request(mt_handle handle, unsigned int *p_slot_id)
{
    CMD_KT_SLOT_REQUEST_S cmd_slot_request;
    mt_s32 ret = MT_FAILURE;

    ret = ioctl((int)handle, KT_IOC_SLOT_REQUEST, &cmd_slot_request);

    if (MT_SUCCESS == ret)
    {
        if (cmd_slot_request.slot_id == MT_KT_SLOT_ID_INVALID)
        {
            *p_slot_id = MT_CIPHER_KEYSLOT_INVALID;
            return MT_FAILURE;
        }

        *p_slot_id = cmd_slot_request.slot_id;
    }

    return ret;
}

mt_s32 mt_mpi_kt_slot_request_multi(mt_handle handle, unsigned int num, unsigned int *p_slot_id)
{
    CMD_KT_SLOT_REQUEST_MULTI_S cmd_slot_request_multi;
    mt_s32 ret = MT_FAILURE;
    mt_s32 i;

    if (num > KT_REQUEST_MULTI_MAX) {
        for (i = 0; i < num; i++)
            p_slot_id[i] = MT_CIPHER_KEYSLOT_INVALID;

        return MT_FAILURE;
    }

    cmd_slot_request_multi.num = num;
    ret = ioctl((int)handle, KT_IOC_SLOT_REQUEST_MULTI, &cmd_slot_request_multi);

    if (MT_SUCCESS == ret)
    {
        for (i = 0; i < num; i++) {
            if (cmd_slot_request_multi.slot_ids[i] == MT_KT_SLOT_ID_INVALID)
                return MT_FAILURE;
        }

        for (i = 0; i < num; i++) {
            p_slot_id[i] = cmd_slot_request_multi.slot_ids[i];
        }
        return MT_SUCCESS;
    }

    return ret;
}

mt_s32 mt_mpi_kt_slot_release(mt_handle handle, unsigned int slot_id)
{
    CMD_KT_SLOT_REQUEST_S cmd_slot_release;

    if (slot_id == MT_CIPHER_KEYSLOT_INVALID)
        return MT_FAILURE;

    cmd_slot_release.slot_id = slot_id;
    
    return ioctl((int)handle, KT_IOC_SLOT_RELEASE, &cmd_slot_release);
}

mt_s32 mt_mpi_kt_slot_active(mt_handle handle, unsigned int slot_id, MT_KT_SLOT_ACTIVE_E active)
{
    CMD_KT_SLOT_ACTIVE_S cmd_slot_active;

    if (slot_id == MT_CIPHER_KEYSLOT_INVALID)
        return MT_FAILURE;

    cmd_slot_active.slot_id = slot_id;
    cmd_slot_active.active = active;
    
    return ioctl((int)handle, KT_IOC_SLOT_ACTIVE, &cmd_slot_active);
}

mt_s32 mt_mpi_kt_attr_config(MT_CIPHER_CTRL_S *p_ctrl, MT_KT_KEY_ATTR_S *p_ka)
{
    mt_s32 ret = MT_SUCCESS;
    MT_KT_SLOT_SIZE_E keysize = MT_KT_SLOT_16B_SIZE;
    MT_KT_SLOT_SIZE_E ivsize = MT_KT_SLOT_16B_SIZE;

    if (p_ctrl == NULL || p_ka == NULL)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    memset(p_ka, 0, sizeof(MT_KT_KEY_ATTR_S));

    switch (p_ctrl->algorithm) {
    case MT_CIPHER_ALG_DES:
        p_ka->DES_ONOFF = MT_KT_ATTR_ON;
        keysize = MT_KT_SLOT_8B_SIZE;
        ivsize = MT_KT_SLOT_8B_SIZE;
        break;
    case MT_CIPHER_ALG_TDES:
        p_ka->TDES_ONOFF = MT_KT_ATTR_ON;
        ivsize = MT_KT_SLOT_8B_SIZE;
        break;
    case MT_CIPHER_ALG_AES:
        p_ka->AES_ONOFF = MT_KT_ATTR_ON;
        break;
    case MT_CIPHER_ALG_SM4:
        p_ka->SM2_3_4_ONOFF = MT_KT_ATTR_ON;
        break;
    case MT_CIPHER_ALG_HMAC:
        p_ka->MAC_ONOFF = MT_KT_ATTR_ON;
        break;
    case MT_CIPHER_ALG_HMAC256:
        p_ka->MAC_ONOFF = MT_KT_ATTR_ON;
        keysize = MT_KT_SLOT_32B_SIZE;
        break;
    case MT_CIPHER_ALG_CSA2:
        p_ka->CSAv2_ONOFF = MT_KT_ATTR_ON;
        keysize = MT_KT_SLOT_8B_SIZE;
        ivsize = MT_KT_SLOT_8B_SIZE;
        break;
    case MT_CIPHER_ALG_CSA3:
        p_ka->CSAv3_ONOFF = MT_KT_ATTR_ON;
        break;
    default:
        ret = MT_FAILURE;
        break;
    }

    p_ka->KEY_SIZE = keysize;
    p_ka->IV_SIZE = ivsize;

    if ((p_ctrl->core == MT_CIPHER_CORE_M2M_RAW)
        || (p_ctrl->core == MT_CIPHER_CORE_M2M_TS)) {
        p_ka->M2M_ONOFF = MT_KT_ATTR_ON;
    }

    if ((p_ctrl->core == MT_CIPHER_CORE_DSC_TS)
        || (p_ctrl->core == MT_CIPHER_CORE_M2M_TS)) {
        p_ka->TS_ONOFF = MT_KT_ATTR_ON;
    }

    if (p_ctrl->operation == MT_CIPHER_OPERATION_DECRYPT) {
        p_ka->DEC_ONOFF = MT_KT_ATTR_ON;
    } else {
        p_ka->ENC_ONOFF = MT_KT_ATTR_ON;
    }

    p_ka->REE_ONOFF = MT_KT_ATTR_ON;

    return ret;
}

mt_s32 mt_mpi_kt_attr_config_ext(MT_KT_CTRL_S *p_ctrl, MT_KT_KEY_ATTR_S *p_ka)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 keysize = 0;
    mt_u32 ivsize = 0;

    if (p_ctrl == NULL || p_ka == NULL)
        return MT_CIPHER_ERR_BAD_PARAMETERS;

    memset(p_ka, 0, sizeof(MT_KT_KEY_ATTR_S));

    if (p_ctrl->purpose) {
        if (p_ctrl->purpose & MT_KT_PURPOSE_TS)
            p_ka->TS_ONOFF = MT_KT_ATTR_ON;

        if (p_ctrl->purpose & MT_KT_PURPOSE_M2M)
            p_ka->M2M_ONOFF = MT_KT_ATTR_ON;

        if (p_ctrl->purpose & MT_KT_PURPOSE_MAC)
            p_ka->MAC_ONOFF = MT_KT_ATTR_ON;
    } else {
        printf("WARN: %s missing purpose\n", __FUNCTION__);
        return MT_FAILURE;
    }

    if (p_ctrl->operation) {
        if (p_ctrl->operation & MT_KT_OPERATION_DECRYPT)
            p_ka->DEC_ONOFF = MT_KT_ATTR_ON;

        if (p_ctrl->operation & MT_KT_OPERATION_ENCRYPT)
            p_ka->ENC_ONOFF = MT_KT_ATTR_ON;
    } else {
        printf("WARN: %s missing operation\n", __FUNCTION__);
        return MT_FAILURE;
    }

    if (p_ctrl->algorithm) {
        if (p_ctrl->algorithm & MT_KT_ALG_DES) {
            p_ka->DES_ONOFF = MT_KT_ATTR_ON;
            if (keysize <MT_KT_SLOT_8B_SIZE)
                keysize = MT_KT_SLOT_8B_SIZE;
            if (ivsize < MT_KT_SLOT_8B_SIZE)
                ivsize = MT_KT_SLOT_8B_SIZE;
        }

        if (p_ctrl->algorithm & MT_KT_ALG_TDES) {
            p_ka->TDES_ONOFF = MT_KT_ATTR_ON;
            if (keysize <MT_KT_SLOT_16B_SIZE)
                keysize = MT_KT_SLOT_16B_SIZE;
            if (ivsize < MT_KT_SLOT_8B_SIZE)
                ivsize = MT_KT_SLOT_8B_SIZE;
        }

        if (p_ctrl->algorithm & MT_KT_ALG_AES) {
            p_ka->AES_ONOFF = MT_KT_ATTR_ON;
            if (keysize <MT_KT_SLOT_16B_SIZE)
                keysize = MT_KT_SLOT_16B_SIZE;
            if (ivsize < MT_KT_SLOT_16B_SIZE)
                ivsize = MT_KT_SLOT_16B_SIZE;
        }

        if (p_ctrl->algorithm & MT_KT_ALG_CSA2) {
            p_ka->CSAv2_ONOFF = MT_KT_ATTR_ON;
            if (keysize <MT_KT_SLOT_8B_SIZE)
                keysize = MT_KT_SLOT_8B_SIZE;
            if (ivsize < MT_KT_SLOT_8B_SIZE)
                ivsize = MT_KT_SLOT_8B_SIZE;
        }

        if (p_ctrl->algorithm & MT_KT_ALG_CSA3) {
            p_ka->CSAv3_ONOFF = MT_KT_ATTR_ON;
            if (keysize <MT_KT_SLOT_16B_SIZE)
                keysize = MT_KT_SLOT_16B_SIZE;
            if (ivsize < MT_KT_SLOT_16B_SIZE)
                ivsize = MT_KT_SLOT_16B_SIZE;
        }

        if (p_ctrl->algorithm & MT_KT_ALG_SM4) {
            p_ka->SM2_3_4_ONOFF = MT_KT_ATTR_ON;
            if (keysize <MT_KT_SLOT_16B_SIZE)
                keysize = MT_KT_SLOT_16B_SIZE;
            if (ivsize < MT_KT_SLOT_16B_SIZE)
                ivsize = MT_KT_SLOT_16B_SIZE;
        }

        if (p_ctrl->algorithm & MT_KT_ALG_GOST28147) {
            p_ka->GOST_28147_89_OR_R34_12_MAGMA_ONOFF = MT_KT_ATTR_ON;
            if (keysize <MT_KT_SLOT_32B_SIZE)
                keysize = MT_KT_SLOT_32B_SIZE;
            if (ivsize < MT_KT_SLOT_8B_SIZE)
                ivsize = MT_KT_SLOT_8B_SIZE;
        }

        if (p_ctrl->algorithm & MT_KT_ALG_GOSTR34) {
            p_ka->GOST_R34_12_KUZNYECHIK_ONOFF = MT_KT_ATTR_ON;
            if (keysize <MT_KT_SLOT_32B_SIZE)
                keysize = MT_KT_SLOT_32B_SIZE;
            if (ivsize < MT_KT_SLOT_16B_SIZE)
                ivsize = MT_KT_SLOT_16B_SIZE;
        }
    } else {
        if (p_ka->MAC_ONOFF == MT_KT_ATTR_OFF) {
            printf("WARN: %s missing algorithm\n", __FUNCTION__);
            return MT_FAILURE;
        }
    }

    if (p_ctrl->keysize) {
        if (p_ctrl->keysize & MT_KT_KEYSIZE_256)
            p_ka->KEY_SIZE = MT_KT_SLOT_32B_SIZE;
        else {
            if (p_ctrl->keysize & MT_KT_KEYSIZE_64)
                p_ka->KEY_SIZE = MT_KT_SLOT_8B_SIZE;

            if (p_ctrl->keysize & MT_KT_KEYSIZE_128)
                p_ka->KEY_SIZE = MT_KT_SLOT_16B_SIZE;
        }
    }

    if (p_ka->KEY_SIZE < keysize) {
        printf("WARN: %s keysize mismatch\n", __FUNCTION__);
    }

    p_ka->IV_SIZE = ivsize;

    p_ka->REE_ONOFF = MT_KT_ATTR_ON;

    return ret;
}

mt_s32 mt_mpi_kt_write_attr(mt_handle handle, unsigned int slot_id, MT_KT_KEY_ATTR_S attr)
{
    CMD_KT_WRITE_ATTR_S cmd_write_attr;

    if (slot_id == MT_CIPHER_KEYSLOT_INVALID)
        return MT_FAILURE;

    cmd_write_attr.slot_id = slot_id;
    cmd_write_attr.attr = attr;
    
    return ioctl((int)handle, KT_IOC_WRITE_ATTR, &cmd_write_attr);
}

mt_s32 mt_mpi_kt_read_attr(mt_handle handle, unsigned int slot_id, MT_KT_KEY_ATTR_S *p_attr)
{
    CMD_KT_READ_ATTR_S cmd_read_attr;
    mt_s32 ret = MT_FAILURE;

    if (slot_id == MT_CIPHER_KEYSLOT_INVALID)
        return MT_FAILURE;

    cmd_read_attr.slot_id = slot_id;
    
    ret = ioctl((int)handle, KT_IOC_READ_ATTR, &cmd_read_attr);

    if (MT_SUCCESS == ret)
    {
        memcpy(p_attr, &(cmd_read_attr.attr), sizeof(MT_KT_KEY_ATTR_S));
    }

    return ret;
}

mt_s32 mt_mpi_kt_write_key(mt_handle handle, unsigned int slot_id, const mt_u8 *p_key, MT_KT_SLOT_SIZE_E size)
{
    CMD_KT_WRITE_KEY_S cmd_write_key;

    if (slot_id == MT_CIPHER_KEYSLOT_INVALID)
        return MT_FAILURE;

    cmd_write_key.slot_id = slot_id;
    memcpy(cmd_write_key.key, p_key, size);
    cmd_write_key.size = size;
    
    return ioctl((int)handle, KT_IOC_WRITE_KEY, &cmd_write_key);
}

mt_s32 mt_mpi_kt_read_key(mt_handle handle, unsigned int slot_id, mt_u8 *p_key, MT_KT_SLOT_SIZE_E size)
{
    CMD_KT_READ_KEY_S cmd_read_key;
    mt_s32 ret = MT_FAILURE;

    if (slot_id == MT_CIPHER_KEYSLOT_INVALID)
        return MT_FAILURE;

    cmd_read_key.slot_id = slot_id;
    cmd_read_key.size = size;
    
    ret = ioctl((int)handle, KT_IOC_READ_KEY, &cmd_read_key);

    if (MT_SUCCESS == ret)
    {
        memcpy(p_key, &(cmd_read_key.key), size);
    }

    return ret;
}

mt_s32 mt_mpi_kt_write_iv(mt_handle handle, unsigned int slot_id, const mt_u8 *p_iv, MT_KT_SLOT_SIZE_E size)
{
    CMD_KT_WRITE_IV_S cmd_write_iv;

    if (slot_id == MT_CIPHER_KEYSLOT_INVALID)
        return MT_FAILURE;

    cmd_write_iv.slot_id = slot_id;
    memcpy(cmd_write_iv.iv, p_iv, size);
    cmd_write_iv.size = size;
    
    return ioctl((int)handle, KT_IOC_WRITE_IV, &cmd_write_iv);
}

mt_s32 mt_mpi_kt_read_iv(mt_handle handle, unsigned int slot_id, mt_u8 *p_iv, MT_KT_SLOT_SIZE_E size)
{
    CMD_KT_READ_IV_S cmd_read_iv;
    mt_s32 ret = MT_FAILURE;

    if (slot_id == MT_CIPHER_KEYSLOT_INVALID)
        return MT_FAILURE;

    cmd_read_iv.slot_id = slot_id;
    cmd_read_iv.size = size;
    
    ret = ioctl((int)handle, KT_IOC_READ_IV, &cmd_read_iv);

    if (MT_SUCCESS == ret)
    {
        memcpy(p_iv, &(cmd_read_iv.iv), size);
    }

    return ret;
}

mt_s32 mt_mpi_kt_read_metadata(mt_handle handle, mt_u32 slot_id, mt_u32 *p_metadata)
{
	CMD_KT_READ_METADATA_S cmd_get_metadata;
	mt_s32 ret = MT_FAILURE;

	if (slot_id == MT_CIPHER_KEYSLOT_INVALID)
		return MT_FAILURE;

	if (!p_metadata)
		return MT_FAILURE;

	cmd_get_metadata.slot_id = slot_id;

	ret = ioctl((mt_s32)handle, KT_IOC_READ_METADATA, &cmd_get_metadata);
	if (MT_SUCCESS == ret)
	{
		*p_metadata = cmd_get_metadata.metadata;
	}

	return ret;
}

mt_s32 mt_mpi_kt_get_state(mt_handle handle, mt_u32 slot_id, MT_KT_SLOT_STATE_E *p_state)
{
	CMD_KT_GET_STATE_S cmd_get_state;
	mt_s32 ret = MT_FAILURE;

	if (slot_id == MT_CIPHER_KEYSLOT_INVALID)
		return MT_FAILURE;

	cmd_get_state.slot_id = slot_id;

	ret = ioctl((mt_s32)handle, KT_IOC_GET_STATE, &cmd_get_state);

	if (MT_SUCCESS == ret)
	{
		*p_state = cmd_get_state.state;
	}

	return ret;
}

mt_s32 mt_mpi_kt_slot_info(mt_handle handle, mt_u32 slot_id)
{
	mt_s32 ret = MT_FAILURE;
	CMD_KT_INFO_S cmd_slot_info;

	if (slot_id == MT_CIPHER_KEYSLOT_INVALID)
		return MT_FAILURE;

	memset(&cmd_slot_info, 0, sizeof(cmd_slot_info));
	cmd_slot_info.slot_id = slot_id;

	ret = ioctl((mt_s32)handle, KT_IOC_SLOT_INFO, &cmd_slot_info);

	if (ret == MT_SUCCESS)
		printf("SLOT[%d] status:%d, valid:0x%x attr:0x%x tee:0x%x\n", slot_id,
			cmd_slot_info.status, cmd_slot_info.valid, cmd_slot_info.attr, cmd_slot_info.teedata);

	return ret;
}

mt_s32 mt_mpi_kt_control_status(mt_handle handle)
{
	mt_s32 ret = MT_FAILURE;
	CMD_KT_CONTROL_S cmd_kt_control;
	int i;

	memset(&cmd_kt_control, 0, sizeof(cmd_kt_control));
	cmd_kt_control.size = 8;
	ret = ioctl((mt_s32)handle, KT_IOC_CONTROL, &cmd_kt_control);
	if (ret == MT_SUCCESS) {
		printf("kt control:");
		for (i = 0; i < cmd_kt_control.size; i++)
			printf(" 0x%x", cmd_kt_control.control[i]);
		printf("\n");
       }

	return ret;
}

