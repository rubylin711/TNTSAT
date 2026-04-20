/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "drv_kt_if.h"
#include "hw_kt_if.h"

#include "mt_mpi_kt.h"

extern void drv_kt_check_private(void *priv);
extern void drv_kt_lock(void *priv);
extern void drv_kt_unlock(void *priv);

#ifdef CONFIG_MT_KEYTABLE_SYM6

#define KT_CONTROL_SLOT HW_KT_SLOT_ID_0

static void drv_kt_control_status(void)
{
    mt_u32 sw_regs[8];
    mt_u32 i;

    i = 8;
    drv_kt_control_get(&i, sw_regs);

    MT_ALWAYS_PRINT("kt status:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
        sw_regs[0],sw_regs[1],sw_regs[2],sw_regs[3],sw_regs[4],sw_regs[5],sw_regs[6],sw_regs[7]);
}

int kt_status_init(void)
{
    mt_u32 sw_regs[8];
    mt_u32 kt_control_state;
    int ret = 0;
    int i;
    HW_KT_SLOT_VALID_E valid = HW_KT_SLOT_INVALIDATE;
    mt_u32 idx, off;

    for (i = 0; i < 8; i++)
        hw_kt_read_sw_reg(&sw_regs[i], i);

    kt_control_state = sw_regs[0] & 3;
    if (kt_control_state == 0) {
        memset(sw_regs, 0, sizeof(sw_regs));
        for (i = HW_KT_SLOT_ID_1; i < HW_KT_SLOT_MAX_NUM; i++) {
            hw_kt_read_valid(i, &valid);
            if (valid == HW_KT_SLOT_VALIDATE) {
                idx = i / 16;
                off = (i - idx * 16) << 1;
                sw_regs[idx] |= (2 << off);
            }
        }
        if (i == HW_KT_SLOT_MAX_NUM) {
            /* kernel init kt control */
            sw_regs[0] |= 2;
            for (i = 7; i >=0; i--)
                hw_kt_write_sw_reg(sw_regs[i], i);
        }
    }
    else if (kt_control_state != 0) {
        ret = 0;
        MT_ALWAYS_PRINT("kt has inited: 0x%x\n", kt_control_state);
    }

    drv_kt_control_status();

    return ret;
}

static int kt_slotid_check(mt_u32 slot_id)
{
    if (slot_id >= HW_KT_SLOT_MAX_NUM)
        return -1;

    if (slot_id == KT_CONTROL_SLOT)
        return -1;

    return 0;
}

static int kt_status_set(mt_u32 id, mt_u32 status)
{
    mt_u32 sw_reg;
    mt_u32 idx, off;

    if (kt_slotid_check(id) < 0)
        return -1;

    idx = id / 16;
    off = (id - idx * 16) << 1;
    hw_kt_read_sw_reg(&sw_reg, idx);

    if (status == HW_KT_SLOT_BUSY)
        sw_reg |= (0x1 << off);
    else
        sw_reg &= ~(0x3<< off);

    hw_kt_write_sw_reg(sw_reg, idx);

    return 0;
}

static int kt_status_get(mt_u32 id, HW_KT_SLOT_STATUS_E *pstatus)
{
    HW_KT_SLOT_VALID_E valid = HW_KT_SLOT_INVALIDATE;
    mt_u32 sw_reg;
    mt_u32 idx, off;
    mt_u32 status;

    if (kt_slotid_check(id) < 0)
        return -1;

    idx = id / 16;
    off = (id - idx * 16) << 1;
    hw_kt_read_sw_reg(&sw_reg, idx);

    status = (sw_reg >> off) & 0x3;
    if (status == 0) {
        hw_kt_read_valid(id, &valid);
        if (valid == HW_KT_SLOT_INVALIDATE) {
            *pstatus = HW_KT_SLOT_FREE;
            return 0;
        }
        sw_reg |= (0x2 << off);
        hw_kt_write_sw_reg(sw_reg, idx);
    }

    *pstatus = HW_KT_SLOT_BUSY;
    return 0;
}

static mt_u32 kt_find_free_slot_multi(mt_u32 start, mt_u32 num)
{
    HW_KT_SLOT_VALID_E valid = HW_KT_SLOT_INVALIDATE;
    mt_u32 sw_regs[8];
    mt_u32 idx, off;
    mt_u32 status;
    mt_u32 i;
    mt_u32 j;
    mt_u32 cnt;

    for (i = 0; i < 8; i++)
        hw_kt_read_sw_reg(&sw_regs[i], i);

    for (i = start; i <= HW_KT_SLOT_MAX_NUM - num; i++) {
        if (i % num == 0) {
            cnt = 0;
            for (j = i; j < i + num; j++) {
                idx = j / 16;
                off = (j - idx * 16) << 1;

                status = (sw_regs[idx] >> off) & 0x3;
                if (status == 0) {
                    hw_kt_read_valid(j, &valid);

                    if (valid == HW_KT_SLOT_INVALIDATE) {
                        hw_kt_write_valid(j, HW_KT_SLOT_VALIDATE);
                        hw_kt_read_valid(j, &valid);
                        if (valid == HW_KT_SLOT_VALIDATE) {
                            hw_kt_write_valid(j, HW_KT_SLOT_INVALIDATE);
                            cnt++;
                            continue;
                        }
                    }
                }
                break;
            }

            if (cnt == num)
                return i;
        }
    }

    MT_ALWAYS_PRINT("can NOT find num:%d slot\n", num);
    return HW_KT_SLOT_ID_INVALID;
}

void drv_kt_slot_request_multi(void *priv, mt_u32 num, mt_u32 *p_slot_id)
{
    mt_u32 slotid = HW_KT_SLOT_ID_INVALID;
    mt_u32 i;

    for (i = 0; i < num; i++)
        p_slot_id[i] = MT_KT_SLOT_ID_INVALID;

    drv_kt_check_private(priv);

    drv_kt_lock(priv);

    slotid = kt_find_free_slot_multi(HW_KT_SLOT_ID_1, num);
    if (slotid != HW_KT_SLOT_ID_INVALID) {
        for (i = 0; i < num; i++) {
            kt_status_set(slotid + i, HW_KT_SLOT_BUSY);
            hw_kt_write_valid(slotid + i, HW_KT_SLOT_VALIDATE);
            p_slot_id[i] = slotid + i;
        }
    }
    drv_kt_unlock(priv);
}

void drv_kt_slot_request(void *priv, mt_u32 *p_slot_id)
{
    return drv_kt_slot_request_multi(priv, 1, p_slot_id);
}

void drv_kt_slot_release(void *priv, mt_u32 slot_id)
{
    uint8_t iv[16] = {0,};

    drv_kt_check_private(priv);

    if (kt_slotid_check(slot_id) < 0)
        return;

    drv_kt_lock(priv);

    hw_kt_write_iv(slot_id, iv, HW_KT_SLOT_16B_SIZE);
    hw_kt_write_valid(slot_id, HW_KT_SLOT_INVALIDATE);
    kt_status_set(slot_id, HW_KT_SLOT_FREE);

    drv_kt_unlock(priv);
}

void drv_kt_slot_active(void *priv, mt_u32 slot_id, MT_KT_SLOT_ACTIVE_E active)
{
    uint32_t k256_err_sta = 0;
    uint32_t k256_err_index = 0;

    drv_kt_check_private(priv);

    if (kt_slotid_check(slot_id) < 0)
        return;

    drv_kt_lock(priv);
    hw_kt_clear_k256_sta();
    hw_kt_write_valid(slot_id, (HW_KT_SLOT_VALID_E)active);
    hw_kt_get_k256_info(&k256_err_sta, &k256_err_index);
    if (k256_err_sta == 1) {
        MT_ERR_KT("%s slot[%d] k256 status error, err_index:%d\n", __FUNCTION__,
            slot_id, k256_err_index);
    }
    drv_kt_unlock(priv);
}

void drv_kt_write_attribute(void *priv, mt_u32 slot_id, MT_KT_KEY_ATTR_S key_attr)
{
    HW_KT_KEY_ATTR_S hw_key_attr;
    uint32_t k256_err_sta = 0;
    uint32_t k256_err_index = 0;

    drv_kt_check_private(priv);

    if (kt_slotid_check(slot_id) < 0)
        return;

    memset(&hw_key_attr, 0, sizeof(HW_KT_KEY_ATTR_S));

    if (key_attr.AES_ONOFF == MT_KT_ATTR_OFF)
        hw_key_attr.bitc.AES_OFFON = 1;

    if (key_attr.DES_ONOFF == MT_KT_ATTR_OFF)
        hw_key_attr.bitc.DES_OFFON = 1;

    if (key_attr.TDES_ONOFF == MT_KT_ATTR_OFF)
        hw_key_attr.bitc.TDES_OFFON = 1;

    if (key_attr.CSAv2_ONOFF == MT_KT_ATTR_OFF)
        hw_key_attr.bitc.CSAv2_OFFON = 1;

    if (key_attr.CSAv3_ONOFF == MT_KT_ATTR_OFF)
        hw_key_attr.bitc.CSAv3_OFFON = 1;

    if (key_attr.ASA_ONOFF == MT_KT_ATTR_OFF)
        hw_key_attr.bitc.ASA_OFFON = 1;

    if (key_attr.SM2_3_4_ONOFF == MT_KT_ATTR_OFF)
        hw_key_attr.bitc.SM4_OFFON = 1;

    if (key_attr.CSAv2_CONFORMANCE_ONOFF == MT_KT_ATTR_OFF)
        hw_key_attr.bitc.CSAv2_CONFORMANCE_OFFON = 1;

    if (key_attr.GOST_28147_89_OR_R34_12_MAGMA_ONOFF == MT_KT_ATTR_OFF)
        hw_key_attr.bitc.GOST_28147_89_OR_R34_12_MAGMA_OFFON = 1;

    if (key_attr.GOST_R34_12_KUZNYECHIK_ONOFF == MT_KT_ATTR_OFF)
        hw_key_attr.bitc.GOST_R34_12_KUZNYECHIK_OFFON = 1;

    if (key_attr.Multi2_ONOFF == MT_KT_ATTR_OFF)
        hw_key_attr.bitc.MULTI2_OFFON = 1;

    if (key_attr.TS_ONOFF == MT_KT_ATTR_OFF)
        hw_key_attr.bitc.TS_OFFON = 1;

    if (key_attr.M2M_ONOFF == MT_KT_ATTR_OFF)
        hw_key_attr.bitc.M2M_OFFON = 1;

    if (key_attr.MAC_ONOFF == MT_KT_ATTR_OFF)
        hw_key_attr.bitc.MAC_OFFON = 1;

    if (key_attr.REE_ONOFF == MT_KT_ATTR_OFF)
        hw_key_attr.bitc.M2M_REE_OFFON = 1;

    if (key_attr.DEC_ONOFF == MT_KT_ATTR_ON)
        hw_key_attr.bitc.DEC_ONOFF = 1;

    if (key_attr.ENC_ONOFF == MT_KT_ATTR_ON)
        hw_key_attr.bitc.ENC_ONOFF = 1;

    hw_key_attr.bitc.KEY_SIZE64_OFFON = 1;
    hw_key_attr.bitc.KEY_SIZE128_OFFON = 1;
    hw_key_attr.bitc.KEY_SIZE256_OFFON = 1;

    if (key_attr.KEY_SIZE == MT_KT_SLOT_8B_SIZE)
        hw_key_attr.bitc.KEY_SIZE64_OFFON = 0;

    if (key_attr.KEY_SIZE == MT_KT_SLOT_16B_SIZE)
        hw_key_attr.bitc.KEY_SIZE128_OFFON = 0;

    if (key_attr.KEY_SIZE == MT_KT_SLOT_32B_SIZE)
        hw_key_attr.bitc.KEY_SIZE256_OFFON = 0;

    drv_kt_lock(priv);
    hw_kt_clear_k256_sta();
    hw_kt_write_attribute(slot_id, hw_key_attr.reg);
    hw_kt_get_k256_info(&k256_err_sta, &k256_err_index);
    if (k256_err_sta == 1) {
        MT_ERR_KT("%s slot[%d] k256 status error, err_index:%d\n", __FUNCTION__,
            slot_id, k256_err_index);
    }
    drv_kt_unlock(priv);
}

void drv_kt_read_attribute(void *priv, mt_u32 slot_id, MT_KT_KEY_ATTR_S *p_attr)
{
    HW_KT_KEY_ATTR_S hw_key_attr = {0};

    drv_kt_check_private(priv);

    if (kt_slotid_check(slot_id) < 0)
        return;

    drv_kt_lock(priv);
    hw_kt_read_attribute(slot_id, &hw_key_attr.reg);
    drv_kt_unlock(priv);

    memset(p_attr, 0, sizeof(MT_KT_KEY_ATTR_S));

    if (hw_key_attr.bitc.AES_OFFON == 0)
        p_attr->AES_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.DES_OFFON == 0)
        p_attr->DES_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.TDES_OFFON == 0)
        p_attr->TDES_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.CSAv2_OFFON == 0)
        p_attr->CSAv2_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.CSAv3_OFFON == 0)
        p_attr->CSAv3_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.ASA_OFFON == 0)
        p_attr->ASA_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.SM4_OFFON == 0)
        p_attr->SM2_3_4_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.CSAv2_CONFORMANCE_OFFON == 0)
        p_attr->CSAv2_CONFORMANCE_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.GOST_28147_89_OR_R34_12_MAGMA_OFFON == 0)
        p_attr->GOST_28147_89_OR_R34_12_MAGMA_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.GOST_R34_12_KUZNYECHIK_OFFON == 0)
        p_attr->GOST_R34_12_KUZNYECHIK_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.MULTI2_OFFON == 0)
        p_attr->Multi2_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.TS_OFFON == 0)
        p_attr->TS_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.M2M_OFFON == 0)
        p_attr->M2M_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.MAC_OFFON == 0)
        p_attr->MAC_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.M2M_REE_OFFON == 0)
        p_attr->REE_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.DEC_ONOFF == 1)
        p_attr->DEC_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.ENC_ONOFF == 1)
        p_attr->ENC_ONOFF = MT_KT_ATTR_ON;

    if (hw_key_attr.bitc.KEY_SIZE64_OFFON == 0)
        p_attr->KEY_SIZE = MT_KT_SLOT_8B_SIZE;

    if (hw_key_attr.bitc.KEY_SIZE128_OFFON == 0)
        p_attr->KEY_SIZE = MT_KT_SLOT_16B_SIZE;

    if (hw_key_attr.bitc.KEY_SIZE256_OFFON == 0)
        p_attr->KEY_SIZE = MT_KT_SLOT_32B_SIZE;

    p_attr->KEY_SOURCE = hw_key_attr.bitc.KEY_SOURCE;
}

void drv_kt_write_key(void *priv, mt_u32 slot_id, mt_u8 *p_key, MT_KT_SLOT_SIZE_E size)
{
    HW_KT_SLOT_STATUS_E status = HW_KT_SLOT_FREE;
    uint32_t k256_err_sta = 0;
    uint32_t k256_err_index = 0;

    drv_kt_check_private(priv);

    if (kt_slotid_check(slot_id) < 0)
        return;

    drv_kt_lock(priv);

    kt_status_get(slot_id, &status);
    if (status == HW_KT_SLOT_BUSY) {
        if (size == MT_KT_SLOT_32B_SIZE) {
            hw_kt_clear_k256_sta();
            hw_kt_write_key256(slot_id, p_key);
            hw_kt_get_k256_info(&k256_err_sta, &k256_err_index);
            if (k256_err_sta == 1) {
                MT_ERR_KT("%s slot[%d] k256 status error, err_index:%d\n", __FUNCTION__,
                    slot_id, k256_err_index);
            }
        }
        else {
            /* write 128bit/64bit key */
            hw_kt_write_key(slot_id, p_key, (HW_KT_SLOT_SIZE_E)size);
        }
    }

    drv_kt_unlock(priv);
}

void drv_kt_read_key(void *priv, mt_u32 slot_id, mt_u8 *p_key, MT_KT_SLOT_SIZE_E size)
{
    drv_kt_check_private(priv);

    if (kt_slotid_check(slot_id) < 0)
        return;

    drv_kt_lock(priv);
    hw_kt_read_key(slot_id, p_key, (HW_KT_SLOT_SIZE_E)size);
    drv_kt_unlock(priv);
}

void drv_kt_write_iv(void *priv, mt_u32 slot_id,  mt_u8 *p_iv, MT_KT_SLOT_SIZE_E size)
{
    uint32_t k256_err_sta = 0;
    uint32_t k256_err_index = 0;

    drv_kt_check_private(priv);

    if (kt_slotid_check(slot_id) < 0)
        return;

    drv_kt_lock(priv);
    hw_kt_clear_k256_sta();
    hw_kt_write_iv(slot_id, p_iv, (HW_KT_SLOT_SIZE_E)size);
    hw_kt_get_k256_info(&k256_err_sta, &k256_err_index);
    if (k256_err_sta == 1) {
        MT_ERR_KT("%s slot[%d] k256 status error, err_index:%d\n", __FUNCTION__,
            slot_id, k256_err_index);
    }
    drv_kt_unlock(priv);
}

void drv_kt_read_iv(void *priv, mt_u32 slot_id, mt_u8 *p_iv, MT_KT_SLOT_SIZE_E size)
{
    drv_kt_check_private(priv);

    if (slot_id == MT_KT_SLOT_ID_INVALID)
        return;

    drv_kt_lock(priv);
    hw_kt_read_iv(slot_id, p_iv, (HW_KT_SLOT_SIZE_E)size);
    drv_kt_unlock(priv);
}

void drv_kt_read_metadata(void *priv, mt_u32 slot_id, mt_u32 *metadata)
{
    drv_kt_check_private(priv);

    if (slot_id == MT_KT_SLOT_ID_INVALID)
        return;

    if (!metadata)
        return;

    drv_kt_lock(priv);
    hw_kt_read_metadata(slot_id, metadata);
    drv_kt_unlock(priv);

}

#endif

#define KT_KA_FORBID 0xbbaa9988
#define KT_KA_DEFAULT 0x0073ffff

void drv_kt_get_state(void *priv, mt_u32 slot_id, MT_KT_SLOT_STATE_E *state)
{
    HW_KT_SLOT_STATUS_E status = HW_KT_SLOT_FREE;
    mt_u32 attr;
    HW_KT_SLOT_VALID_E valid_out;
    mt_u32 temp;

    if (slot_id >= HW_KT_SLOT_MAX_NUM)
        return ;

    drv_kt_check_private(priv);

    drv_kt_lock(priv);
    kt_status_get(slot_id, &status);
    hw_kt_read_valid(slot_id, &valid_out);
    hw_kt_read_attribute(slot_id, &attr);
    drv_kt_unlock(priv);

    if (status == HW_KT_SLOT_FREE) {
        *state = MT_KT_STATE_INVALID;
        return;
    }

    if (valid_out == HW_KT_SLOT_INVALIDATE) {
        *state = MT_KT_STATE_UNINIT;
        return;
    }

    temp = *(mt_u32 *)&attr;
    if (temp == KT_KA_DEFAULT) {
        *state = MT_KT_STATE_UNINIT;
        return;
    }
    else if (temp == KT_KA_FORBID) {
        *state = MT_KT_STATE_READ_DIS;
        return;
    }

    *state = MT_KT_STATE_INITED;
    return;
}

void drv_kt_slot_info(mt_u32 slot_id)
{
    HW_KT_SLOT_STATUS_E status = HW_KT_SLOT_FREE;
    mt_u32 attr;
    HW_KT_SLOT_VALID_E valid_out;
    mt_u32 teedata;

    if (slot_id >= HW_KT_SLOT_MAX_NUM)
        return ;

    kt_status_get(slot_id, &status);
    hw_kt_read_valid(slot_id, &valid_out);
    hw_kt_read_attribute(slot_id, &attr);
    hw_kt_read_tee(slot_id, &teedata);
    printk("SLOT[%d] status:%d valid:0x%x attr:0x%x tee:0x%x\n",
       slot_id, status, valid_out, attr, teedata);
}

void drv_kt_slot_info_get(void *priv, mt_u32 slot_id,
    mt_u32 *status, mt_u32 *valid, mt_u32 *attr, mt_u32 *teedata)
{
    if (!status || !valid || !attr || !teedata)
        return;

    if (slot_id >= HW_KT_SLOT_MAX_NUM)
        return ;

    drv_kt_check_private(priv);

    drv_kt_lock(priv);
    kt_status_get(slot_id, (HW_KT_SLOT_STATUS_E *)status);
    hw_kt_read_valid(slot_id, (HW_KT_SLOT_VALID_E *)valid);
    hw_kt_read_attribute(slot_id, attr);
    hw_kt_read_tee(slot_id, teedata);
    drv_kt_unlock(priv);
}

void drv_kt_control_get(mt_u32 *size, mt_u32 *control)
{
    mt_u32 i;

    if (!size || !control)
        return;

    if (*size < 8)
        return;

    for (i = 0; i < 8; i++)
        hw_kt_read_sw_reg(&control[i], i);

    *size = 8;
}

