/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"
#include "hw_kt_register.h"
#include "hw_kt_if.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

/***************************extern function *************************/

typedef union _HW_KT_KEY_ATTR_REG
{
    mt_u32 all;
    struct 
    {
        mt_u32 AES_OFFON: 1;
        mt_u32 DES_OFFON: 1;
        mt_u32 TDES_OFFON: 1;
        mt_u32 CSAv2_OFFON: 1;
        mt_u32 CSAv3_OFFON: 1;
        mt_u32 resv1: 9;
        mt_u32 TS_OFFON: 1;
        mt_u32 M2M_OFFON: 1;
        mt_u32 MAC_OFFON: 1;
        mt_u32 M2M_REE_OFFON: 1;
        mt_u32 DEC_ONOFF: 1;
        mt_u32 ENC_ONOFF: 1;
        mt_u32 KEY_SIZE64_OFFON: 1;
        mt_u32 KEY_SIZE128_OFFON: 1;
        mt_u32 resv2: 6;
        //0000: PortACPUKey
        //0001: PortSCPUKey
        //0010: PortVSCPUKey
        //0011: PortVSCPUCAV1Key
        //0100: PortACPUCAV1Key
        //0101: PortSCPUCAV1Key
        //0110: PortACPUCAV2Key
        //0111: PortSCPUCAV2Key
        //1000: PortACPUCAV3Key
        //1001: PortSCPUCAV3Key
        mt_u32 KEY_SOURCE: 4;
    } bitc;
}HW_KT_KEY_ATTR_REG;

typedef union _HW_KT_TEE_REG
{
    mt_u32 all;
    struct {
        mt_u32 TEE_PRM:4;
        mt_u32 TEE_WCID:4;
        mt_u32 TEE_SCID:4;
        mt_u32 TEE_SC:1;
        mt_u32 TEE_TP:1;
        mt_u32 TEE_ENC:1;
        mt_u32 TEE_DEC:1;
        mt_u32 TEE_AUDIO:1;
        mt_u32 :15;
    } bitc;
}HW_KT_TEE_REG;

typedef union _HW_TEE_CFG_DATA_CHK_REG1
{
    mt_u32 all;
    struct {
        mt_u32 invalid_keyslot: 1;
        mt_u32 chk_field_1t31: 31;
    } bitc;
}HW_TEE_CFG_DATA_CHK_REG1;

typedef union _HW_TEE_CFG_DATA_CHK_REG2
{
    mt_u32 all;
    struct {
        mt_u32 chk_field_32t35: 4;
        mt_u32 resv: 28;
    } bitc;
}HW_TEE_CFG_DATA_CHK_REG2;

static void memcpy_from_reg(unsigned char *dest, const unsigned char *reg, int len)
{
    int i = 0;
    unsigned long value = 0;
    for(i=0;i<len/4;i++)
    {
        value = *(volatile unsigned long *)(reg+i*4);
        dest[i*4+0] = (value >> 0) & 0xFF;
        dest[i*4+1] = (value >> 8) & 0xFF;
        dest[i*4+2] = (value >> 16) & 0xFF;
        dest[i*4+3] = (value >> 24) & 0xFF;
    }
}

static void memcpy_to_reg(unsigned char *reg, const unsigned char *src, int len)
{
    int i = 0;
    unsigned long value = 0;
    for(i=0;i<len/4;i++)
    {
        value = ((unsigned long)src[i*4+0] << 0) | ((unsigned long)src[i*4+1] << 8) | ((unsigned long)src[i*4+2] << 16) | ((unsigned long)src[i*4+3] << 24);
        *(volatile unsigned long *)(reg+i*4) = value;
    }
}

void hw_kt_write_valid(HW_KT_SLOT_ID_E slot_id, HW_KT_SLOT_VALID_E valid)
{
    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    //config big endian
    HAL_PUT_U32((volatile mt_u32 *)KT_ENDIAN, 0x01);
    HAL_PUT_U32((volatile mt_u32 *)KT_OPERATION, HW_KT_OPER_VALID|(slot_id << 8));
    HAL_PUT_U32((volatile mt_u32 *)KT_WR_DATA_31T0, valid);

    //trigger a operation
    HAL_PUT_U32((volatile mt_u32 *)KT_START, 0x01);

    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);
}

void hw_kt_write_attribute(HW_KT_SLOT_ID_E slot_id, HW_KT_KEY_ATTR_S key_attr)
{
    HW_KT_KEY_ATTR_REG key_attr_reg = {0};

    key_attr_reg.bitc.AES_OFFON = key_attr.AES_OFFON;
    key_attr_reg.bitc.DES_OFFON = key_attr.DES_OFFON;
    key_attr_reg.bitc.TDES_OFFON = key_attr.TDES_OFFON;
    key_attr_reg.bitc.CSAv2_OFFON = key_attr.CSAv2_OFFON;
    key_attr_reg.bitc.CSAv3_OFFON = key_attr.CSAv3_OFFON;
    key_attr_reg.bitc.TS_OFFON = key_attr.TS_OFFON;
    key_attr_reg.bitc.M2M_OFFON = key_attr.M2M_OFFON;
    key_attr_reg.bitc.MAC_OFFON = key_attr.MAC_OFFON;
    key_attr_reg.bitc.M2M_REE_OFFON = key_attr.M2M_REE_OFFON;
    key_attr_reg.bitc.DEC_ONOFF = key_attr.DEC_ONOFF;
    key_attr_reg.bitc.ENC_ONOFF = key_attr.ENC_ONOFF;
    key_attr_reg.bitc.KEY_SIZE64_OFFON = key_attr.KEY_SIZE64_OFFON;
    key_attr_reg.bitc.KEY_SIZE128_OFFON = key_attr.KEY_SIZE128_OFFON;
    key_attr_reg.bitc.KEY_SOURCE = key_attr.KEY_SOURCE;
    key_attr_reg.bitc.resv1 = key_attr.resv1;
    key_attr_reg.bitc.resv2 = key_attr.resv2;
    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    //config big endian
    HAL_PUT_U32((volatile mt_u32 *)KT_ENDIAN, 0x1);
    HAL_PUT_U32((volatile mt_u32 *)KT_OPERATION, HW_KT_OPER_W_ATTR| (slot_id << 8));
    HAL_PUT_U32((volatile mt_u32 *)KT_WR_DATA_31T0, key_attr_reg.all);

    //trigger a operation
    HAL_PUT_U32((volatile mt_u32 *)KT_START, 0x01);

    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);
}

void hw_kt_write_tee(HW_KT_SLOT_ID_E slot_id, HW_KT_TEE_S tee_cfg)
{
    HW_KT_TEE_REG tee_cfg_reg = {0};

    tee_cfg_reg.bitc.TEE_PRM = tee_cfg.TEE_PRM;
    tee_cfg_reg.bitc.TEE_WCID = tee_cfg.TEE_WCID;
    tee_cfg_reg.bitc.TEE_SCID = tee_cfg.TEE_SCID;
    tee_cfg_reg.bitc.TEE_SC = tee_cfg.TEE_SC;
    tee_cfg_reg.bitc.TEE_TP = tee_cfg.TEE_TP;
    tee_cfg_reg.bitc.TEE_ENC = tee_cfg.TEE_ENC;
    tee_cfg_reg.bitc.TEE_DEC = tee_cfg.TEE_DEC;
    tee_cfg_reg.bitc.TEE_AUDIO = tee_cfg.TEE_AUDIO;

    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    //config big endian
    HAL_PUT_U32((volatile mt_u32 *)KT_ENDIAN, 0x01);
    HAL_PUT_U32((volatile mt_u32 *)KT_OPERATION, HW_KT_OPER_W_TEE|(slot_id << 8));
    HAL_PUT_U32((volatile mt_u32 *)KT_WR_DATA_31T0, tee_cfg_reg.all);

    //trigger a operation
    HAL_PUT_U32((volatile mt_u32 *)KT_START, 0x01);

    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);
}

void hw_kt_write_key(HW_KT_SLOT_ID_E slot_id, const mt_u8 *p_key, HW_KT_SLOT_SIZE_E size)
{  
    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    //config little endian
    HAL_PUT_U32((volatile mt_u32 *)KT_ENDIAN, 0x0);
    HAL_PUT_U32((volatile mt_u32 *)KT_OPERATION, HW_KT_OPER_W_KEY|(slot_id << 8));

    if (size == HW_KT_SLOT_8B_SIZE)
        memcpy_to_reg((mt_u8 *)KT_WR_DATA_63T32, p_key, size);
    else
        memcpy_to_reg((mt_u8 *)KT_WR_DATA_127T96, p_key, size);

    //trigger a operation
    HAL_PUT_U32((volatile mt_u32 *)KT_START, 0x01);

    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);
}

void hw_kt_write_iv(HW_KT_SLOT_ID_E slot_id, const mt_u8 *p_iv, HW_KT_SLOT_SIZE_E size)
{ 
    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    //config little endian
    HAL_PUT_U32((volatile mt_u32 *)KT_ENDIAN, 0x0);
    HAL_PUT_U32((volatile mt_u32 *)KT_OPERATION, HW_KT_OPER_W_IV|(slot_id << 8));

    if (size == HW_KT_SLOT_8B_SIZE)
        memcpy_to_reg((mt_u8 *)KT_WR_DATA_63T32, p_iv, size);
    else
        memcpy_to_reg((mt_u8 *)KT_WR_DATA_127T96, p_iv, size);

    //trigger a operation
    HAL_PUT_U32((volatile mt_u32 *)KT_START, 0x01);

    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);
}

void hw_kt_read_valid(HW_KT_SLOT_ID_E slot_id, HW_KT_SLOT_VALID_E *p_valid)
{ 
    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    //config little endian
    HAL_PUT_U32((volatile mt_u32 *)KT_ENDIAN, 0x0);
    HAL_PUT_U32((volatile mt_u32 *)KT_OPERATION, HW_KT_OPER_R|(slot_id << 8));

    //trigger a operation
    HAL_PUT_U32((volatile mt_u32 *)KT_START, 0x01);

    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    *p_valid = HAL_GET_U32((volatile mt_u32 *)KT_RD_DATA_SLOTVALID) & 0x1;

    //printk("slot %d: %s\n", slot_id, *p_valid ? "Valid" : "Invalid");
}

void hw_kt_read_attribute(HW_KT_SLOT_ID_E slot_id, HW_KT_KEY_ATTR_S *p_attr)
{ 
    HW_KT_KEY_ATTR_REG key_attr_reg;

    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    //config little endian
    HAL_PUT_U32((volatile mt_u32 *)KT_ENDIAN, 0x0);
    HAL_PUT_U32((volatile mt_u32 *)KT_OPERATION, HW_KT_OPER_R|(slot_id << 8));

    //trigger a operation
    HAL_PUT_U32((volatile mt_u32 *)KT_START, 0x01);

    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    key_attr_reg.all = HAL_GET_U32((volatile mt_u32 *)KT_RD_DATA_ATTR);

    memset(p_attr, 0, sizeof(HW_KT_KEY_ATTR_S));
    p_attr->AES_OFFON = key_attr_reg.bitc.AES_OFFON;
    p_attr->DES_OFFON = key_attr_reg.bitc.DES_OFFON;
    p_attr->TDES_OFFON = key_attr_reg.bitc.TDES_OFFON;
    p_attr->CSAv2_OFFON = key_attr_reg.bitc.CSAv2_OFFON;
    p_attr->CSAv3_OFFON = key_attr_reg.bitc.CSAv3_OFFON;
    p_attr->TS_OFFON = key_attr_reg.bitc.TS_OFFON;
    p_attr->M2M_OFFON = key_attr_reg.bitc.M2M_OFFON;
    p_attr->MAC_OFFON = key_attr_reg.bitc.MAC_OFFON;
    p_attr->M2M_REE_OFFON = key_attr_reg.bitc.M2M_REE_OFFON;
    p_attr->DEC_ONOFF = key_attr_reg.bitc.DEC_ONOFF;
    p_attr->ENC_ONOFF = key_attr_reg.bitc.ENC_ONOFF;
    p_attr->KEY_SIZE64_OFFON = key_attr_reg.bitc.KEY_SIZE64_OFFON;
    p_attr->KEY_SIZE128_OFFON = key_attr_reg.bitc.KEY_SIZE128_OFFON;
    p_attr->KEY_SOURCE = key_attr_reg.bitc.KEY_SOURCE;
    p_attr->resv1 = key_attr_reg.bitc.resv1;
    p_attr->resv2 = key_attr_reg.bitc.resv2;
}

void hw_kt_read_tee_chk(HW_KT_SLOT_ID_E slot_id, HW_KT_TEE_DATA_CHK_E *p_chk)
{
    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    //config little endian
    HAL_PUT_U32((volatile mt_u32 *)KT_ENDIAN, 0x0);
    HAL_PUT_U32((volatile mt_u32 *)KT_OPERATION, HW_KT_OPER_R|(slot_id << 8));

    //trigger a operation
    HAL_PUT_U32((volatile mt_u32 *)KT_START, 0x01);

    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    *p_chk = (HAL_GET_U32((volatile mt_u32 *)KT_RD_DATA_SLOTVALID) >> 31) & 0x1;
}

void hw_kt_read_tee(HW_KT_SLOT_ID_E slot_id, HW_KT_TEE_S *p_tee)
{ 
    HW_KT_TEE_REG tee_reg;

    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    //config little endian
    HAL_PUT_U32((volatile mt_u32 *)KT_ENDIAN, 0x0);
    HAL_PUT_U32((volatile mt_u32 *)KT_OPERATION, HW_KT_OPER_R_TEE|(slot_id << 8));

    //trigger a operation
    HAL_PUT_U32((volatile mt_u32 *)KT_START, 0x01);

    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    tee_reg.all = HAL_GET_U32((volatile mt_u32 *)KT_RD_DATA_TEE);

    memset(p_tee, 0, sizeof(HW_KT_TEE_S));
    p_tee->TEE_PRM = tee_reg.bitc.TEE_PRM;
    p_tee->TEE_WCID = tee_reg.bitc.TEE_WCID;
    p_tee->TEE_SCID = tee_reg.bitc.TEE_SCID;
    p_tee->TEE_SC = tee_reg.bitc.TEE_SC;
    p_tee->TEE_TP = tee_reg.bitc.TEE_TP;
    p_tee->TEE_ENC = tee_reg.bitc.TEE_ENC;
    p_tee->TEE_DEC = tee_reg.bitc.TEE_DEC;
    p_tee->TEE_AUDIO = tee_reg.bitc.TEE_AUDIO;
}

void hw_kt_read_iv(HW_KT_SLOT_ID_E slot_id, mt_u8 *p_iv, HW_KT_SLOT_SIZE_E size)
{ 
    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    //config little endian
    HAL_PUT_U32((volatile mt_u32 *)KT_ENDIAN, 0x0);
    HAL_PUT_U32((volatile mt_u32 *)KT_OPERATION, HW_KT_OPER_R|(slot_id << 8));

    //trigger a operation
    HAL_PUT_U32((volatile mt_u32 *)KT_START, 0x01);

    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    if (size == HW_KT_SLOT_8B_SIZE)
        memcpy_from_reg(p_iv, (mt_u8 *)KT_RD_DATA_63T32, size);
    else
        memcpy_from_reg(p_iv, (mt_u8 *)KT_RD_DATA_127T96, size);
}

void hw_kt_read_key(HW_KT_SLOT_ID_E slot_id, mt_u8 *p_key, HW_KT_SLOT_SIZE_E size)
{ 
    HAL_PUT_U32((volatile mt_u32 *)KT_RD_KEY_DEBUG, 1);

    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    //config little endian
    HAL_PUT_U32((volatile mt_u32 *)KT_ENDIAN, 0x0);
    HAL_PUT_U32((volatile mt_u32 *)KT_OPERATION, HW_KT_OPER_R|(slot_id << 8));

    //trigger a operation
    HAL_PUT_U32((volatile mt_u32 *)KT_START, 0x01);

    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)KT_START) & 1);

    if (size == HW_KT_SLOT_8B_SIZE)
        memcpy_from_reg(p_key, (mt_u8 *)KT_RD_KEY_63T32, size);
    else
        memcpy_from_reg(p_key, (mt_u8 *)KT_RD_KEY_127T96, size);
}

//Nonce 的IV固定为0
//Nonce 的Attr固定为0xF0187FFE(Ree Permission)或者0xF01A7FFE(TEE Permission)
//在开始接收新的Nonce时候(或KT_RNG_CLR bit[0]写1), 清除SlotValid和Key
//在接收完一个完整的Nonce之后,SlotValid置高Key有效
void hw_kt_nonce_clear(void)
{
    HAL_PUT_U32((volatile mt_u32 *)KT_RNG_CLR, 0x1);
}

void hw_kt_get_nonce_status(HW_KT_NONCE_STATUS_E *p_stat)
{
    *p_stat = (HAL_GET_U32((volatile mt_u32 *)KT_RNG_STATUS)>>4)&0x1;
}

static HW_KT_SLOT_STATUS_E keyslot_status[HW_KT_SLOT_MAX_NUM] = {HW_KT_SLOT_FREE};

HW_KT_SLOT_ID_E hw_kt_request()
{
    mt_u32 i = 0;

    for(i = HW_KT_SLOT_ID_2; i < HW_KT_SLOT_MAX_NUM; i ++)
    {
        if(keyslot_status[i] == HW_KT_SLOT_FREE)
            break;       
    }

    if(i < HW_KT_SLOT_MAX_NUM)
    {
        keyslot_status[i] = HW_KT_SLOT_BUSY;
        return i;
    }

    return HW_KT_SLOT_ID_INVALID;
}

void hw_kt_release(HW_KT_SLOT_ID_E slot_id)
{
    if (HW_KT_SLOT_BUSY == keyslot_status[slot_id])
    {
        hw_kt_write_valid(slot_id, HW_KT_SLOT_INVALIDATE);
        keyslot_status[slot_id] = HW_KT_SLOT_FREE;
    }
    else
    {
        printk("slot id %d is already free, so do nothing\n",slot_id);
    }
}

void hw_kt_write_tee_cfgdata_checken(HW_TEE_CFG_DATA_CHKEN_E enable)
{
    unsigned int data;
    data = HAL_GET_U32((volatile mt_u32 *)KT_IVCLR_MODE);
    data &= ~(1<<4);
    data |= (enable & 0x1) << 4;
    HAL_PUT_U32((volatile mt_u32 *)KT_IVCLR_MODE, data);
}

void hw_kt_read_tee_cfgdata_checken(HW_TEE_CFG_DATA_CHKEN_E *p_enable)
{
    *p_enable = (HAL_GET_U32((volatile mt_u32 *)KT_IVCLR_MODE) >> 4) & 1;
}

void hw_kt_write_tee_cfgdata_chk_lock(HW_TEE_CFG_DATA_CHK_LOCK_E lock)
{
    unsigned int data;
    data = HAL_GET_U32((volatile mt_u32 *)KT_WR_TEE_CFG_DATA_CHK_LOCK);
    data &= ~(1<<0);
    data |= (lock & 0x1) << 0;
    HAL_PUT_U32((volatile mt_u32 *)KT_WR_TEE_CFG_DATA_CHK_LOCK, data);
}

void hw_kt_read_tee_cfgdata_chk_lock(HW_TEE_CFG_DATA_CHK_LOCK_E *p_lock)
{
    *p_lock = (HAL_GET_U32((volatile mt_u32 *)KT_WR_TEE_CFG_DATA_CHK_LOCK) >> 0) & 1;
}

void hw_kt_write_tee_cfgdata_chk_invalid(HW_TEE_CFG_DATA_CHK_INVALID_E invalid)
{
    HW_TEE_CFG_DATA_CHK_REG1 reg1 = {0};
    reg1.all = HAL_GET_U32((volatile mt_u32 *)KT_WR_TEE_CFG_DATA_CHK_31t0);
    reg1.bitc.invalid_keyslot = invalid;
    HAL_PUT_U32((volatile mt_u32 *)KT_WR_TEE_CFG_DATA_CHK_31t0, reg1.all);
}

void hw_kt_read_tee_cfgdata_chk_invalid(HW_TEE_CFG_DATA_CHK_INVALID_E *p_invalid)
{
    HW_TEE_CFG_DATA_CHK_REG1 reg1 = {0};
    reg1.all = HAL_GET_U32((volatile mt_u32 *)KT_WR_TEE_CFG_DATA_CHK_31t0);
    *p_invalid = reg1.bitc.invalid_keyslot;
}

void hw_kt_write_tee_cfgdata_chk(HW_TEE_CFG_DATA_CHK_S *p_check)
{
    HW_TEE_CFG_DATA_CHK_REG1 reg1 = {0};
    HW_TEE_CFG_DATA_CHK_REG2 reg2 = {0};
    reg1.bitc.invalid_keyslot = p_check->invalid_keyslot;
    reg1.bitc.chk_field_1t31 = p_check->chk_field_1t31;
    reg2.bitc.chk_field_32t35 = p_check->chk_field_32t35;

    HAL_PUT_U32((volatile mt_u32 *)KT_WR_TEE_CFG_DATA_CHK_31t0, reg1.all);
    HAL_PUT_U32((volatile mt_u32 *)KT_WR_TEE_CFG_DATA_CHK_35t32, reg2.all);
}

void hw_kt_read_tee_cfgdata_chk(HW_TEE_CFG_DATA_CHK_S *p_check)
{
    HW_TEE_CFG_DATA_CHK_REG1 reg1 = {0};
    HW_TEE_CFG_DATA_CHK_REG2 reg2 = {0};
    reg1.all = HAL_GET_U32((volatile mt_u32 *)KT_WR_TEE_CFG_DATA_CHK_31t0);
    reg2.all = HAL_GET_U32((volatile mt_u32 *)KT_WR_TEE_CFG_DATA_CHK_35t32);
    p_check->invalid_keyslot = reg1.bitc.invalid_keyslot;
    p_check->chk_field_1t31 = reg1.bitc.chk_field_1t31;
    p_check->chk_field_32t35 = reg2.bitc.chk_field_32t35;
}

