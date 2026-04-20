/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"
#include "hw_kt_register.h"
#include "hw_kt_if.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

static void memcpy_from_reg(unsigned char *dest, unsigned char *reg, int len)
{
    int i = 0;
    unsigned long value = 0;
    for (i = 0; i < len / 4; i++) {
		value = HAL_GET_U32((volatile u32 *)(reg + i * 4));
		dest[i * 4 + 0] = (value >> 0) & 0xFF;
		dest[i * 4 + 1] = (value >> 8) & 0xFF;
		dest[i * 4 + 2] = (value >> 16) & 0xFF;
		dest[i * 4 + 3] = (value >> 24) & 0xFF;
    }
}

static void memcpy_to_reg(unsigned char *reg, unsigned char *src, int len)
{
    int i = 0;
    unsigned long value = 0;
    for (i = 0; i < len / 4; i++) {
		value = ((unsigned long)src[i * 4 + 0] << 0) | ((unsigned long)src[i * 4 + 1] << 8) | ((unsigned long)src[i * 4 + 2] << 16) | ((unsigned long)src[i * 4 + 3] << 24);
		HAL_PUT_U32((volatile u32 *)(reg + i * 4), value);
    }
}

void hw_kt_write_valid(HW_KT_SLOT_ID_E slot_id, HW_KT_SLOT_VALID_E valid)
{
    //wait bit0 clear
    while (HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_START)) & 1)
	;

    //config big endian
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_ENDIAN_CONFIG), 0x01);
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_OPERATION), HW_KT_OPER_ACTIVE | (slot_id << 8));
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_WRITE_DATA3), valid);

    //trigger a operation
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_START), 0x01);

    //wait bit0 clear
    while (HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_START)) & 1)
	;
}

void hw_kt_read_valid(HW_KT_SLOT_ID_E slot_id, HW_KT_SLOT_VALID_E *p_valid)
{
    //wait bit0 clear
    while(HAL_GET_U32((volatile u32 *)REG_KEYTABLE_START) & 1)
        ;

    //config little endian
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_ENDIAN_CONFIG), 0x0);
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_OPERATION), HW_KT_OPER_R | (slot_id << 8));

    //trigger a operation
    HAL_PUT_U32((volatile mt_u32 *)REG_KEYTABLE_START, 0x01);

    //wait bit0 clear
    while(HAL_GET_U32((volatile mt_u32 *)REG_KEYTABLE_START) & 1);

    *p_valid = HAL_GET_U32((volatile mt_u32 *)REG_KEYTABLE_KEYSLOVALID) & 0x1;
}

void hw_kt_write_attribute(HW_KT_SLOT_ID_E slot_id, HW_KT_KEY_ATTR_S key_attr)
{
    HW_KT_KEY_ATTR_REG key_attr_reg;

    key_attr_reg.bitc.AES_ONOFF = key_attr.AES_ONOFF;
    key_attr_reg.bitc.DES_ONOFF = key_attr.DES_ONOFF;
    key_attr_reg.bitc.TDES_ONOFF = key_attr.TDES_ONOFF;
    key_attr_reg.bitc.CSAv2_ONOFF = key_attr.CSAv2_ONOFF;
    key_attr_reg.bitc.CSAv3_ONOFF = key_attr.CSAv3_ONOFF;
    key_attr_reg.bitc.SM2_3_4_ONOFF = key_attr.SM2_3_4_ONOFF;
    key_attr_reg.bitc.HMAC_ONOFF = key_attr.HMAC_ONOFF;
    key_attr_reg.bitc.M2M_ONOFF = key_attr.M2M_ONOFF;
    key_attr_reg.bitc.ASA_ONOFF = key_attr.ASA_ONOFF;
    key_attr_reg.bitc.Multi2_ONOFF = key_attr.Multi2_ONOFF;
    key_attr_reg.bitc.REE_ONOFF = key_attr.REE_ONOFF;
    key_attr_reg.bitc.DEC_ONOFF = key_attr.DEC_ONOFF;
    key_attr_reg.bitc.ENC_ONOFF = key_attr.ENC_ONOFF;
    key_attr_reg.bitc.KEY_SIZE = key_attr.KEY_SIZE;
    key_attr_reg.bitc.KEY_SOURCE = key_attr.KEY_SOURCE;
    key_attr_reg.bitc.TDES_KEYCHK = key_attr.TDES_KEYCHK;

    /*
    printk("AES(%x)-DES(%x)-TDES(%x)-CSAv2(%x)-CSAv3(%x)-M2M(%x)-DEC(%x)-ENC(%x)-kSize(%x)-KSRC(%x)\n",
           key_attr_reg.bitc.AES_ONOFF,
           key_attr_reg.bitc.DES_ONOFF,
           key_attr_reg.bitc.TDES_ONOFF,
           key_attr_reg.bitc.CSAv2_ONOFF,
           key_attr_reg.bitc.CSAv3_ONOFF,
           key_attr_reg.bitc.M2M_ONOFF,
           key_attr_reg.bitc.DEC_ONOFF,
           key_attr_reg.bitc.ENC_ONOFF,
           key_attr_reg.bitc.KEY_SIZE,
           key_attr_reg.bitc.KEY_SOURCE);
    */

    //wait bit0 clear
    while (HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_START)) & 1)
	;

    //config big endian
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_ENDIAN_CONFIG), 0x1);
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_OPERATION), HW_KT_OPER_W_ATTRIBUTE | (slot_id << 8));
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_WRITE_DATA3), key_attr_reg.all);

    //trigger a operation
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_START), 0x01);

    //wait bit0 clear
    while (HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_START)) & 1)
	;

    //hal_symphony2_kt_slot_active(slot_id, OPER_VALIDATE);
}

void hw_kt_read_attribute(HW_KT_SLOT_ID_E slot_id, HW_KT_KEY_ATTR_S *p_attr)
{
    HW_KT_KEY_ATTR_REG key_attr_reg;

    //wait bit0 clear
    while (HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_START)) & 1)
	;

    //config little endian
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_ENDIAN_CONFIG), 0x0);
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_OPERATION), HW_KT_OPER_R | (slot_id << 8));

    //trigger a operation
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_START), 0x01);

    //wait bit0 clear	????????????????????????????????
    while (HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_START)) & 1)
	;

    key_attr_reg.all = HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_KEYATTRIBUTE));

    p_attr->AES_ONOFF = key_attr_reg.bitc.AES_ONOFF;
    p_attr->DES_ONOFF = key_attr_reg.bitc.DES_ONOFF;
    p_attr->TDES_ONOFF = key_attr_reg.bitc.TDES_ONOFF;
    p_attr->CSAv2_ONOFF = key_attr_reg.bitc.CSAv2_ONOFF;
    p_attr->CSAv3_ONOFF = key_attr_reg.bitc.CSAv3_ONOFF;
    p_attr->SM2_3_4_ONOFF = key_attr_reg.bitc.SM2_3_4_ONOFF;
    p_attr->HMAC_ONOFF = key_attr_reg.bitc.HMAC_ONOFF;
    p_attr->M2M_ONOFF = key_attr_reg.bitc.M2M_ONOFF;
    p_attr->ASA_ONOFF = key_attr_reg.bitc.ASA_ONOFF;
    p_attr->Multi2_ONOFF = key_attr_reg.bitc.Multi2_ONOFF;
    p_attr->REE_ONOFF = key_attr_reg.bitc.REE_ONOFF;
    p_attr->DEC_ONOFF = key_attr_reg.bitc.DEC_ONOFF;
    p_attr->ENC_ONOFF = key_attr_reg.bitc.ENC_ONOFF;
    p_attr->KEY_SIZE = key_attr_reg.bitc.KEY_SIZE;
    p_attr->KEY_SOURCE = key_attr_reg.bitc.KEY_SOURCE;
    p_attr->TDES_KEYCHK = key_attr_reg.bitc.TDES_KEYCHK;

    //hal_symphony2_kt_slot_active(slot_id, OPER_VALIDATE);
}

/*
static void dump_data(u8 *data_src, u32 len)
{
    u32 i = 0;

    printk("\n");
    for (i = 0; i < len; i++) {
	printk("%x ", data_src[i]);
	if ((i + 1) % 16 == 0) {
	    printk("\n");
	}
    }
    printk("\n");
}
*/

void hw_kt_write_key(HW_KT_SLOT_ID_E slot_id, u8 *p_key, HW_KT_SLOT_SIZE_E size)
{
    //dump_data(p_key, size);

    //wait bit0 clear
    while (HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_START)) & 1)
	;

    //config little endian
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_ENDIAN_CONFIG), 0x0);
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_OPERATION), HW_KT_OPER_W_KEY | (slot_id << 8));

    if (HW_KT_SLOT_8B_SIZE == size) //8 bit
    {
	memcpy_to_reg((u8 *)REG_KEYTABLE_WRITE_DATA2, p_key, 8);
    } else //16bit
    {
	memcpy_to_reg((u8 *)REG_KEYTABLE_WRITE_DATA0, p_key, 16);
    }

    //printk("addr(0x%x) = 0x%x\n", REG_KEYTABLE_ENDIAN_CONFIG, HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_ENDIAN_CONFIG)));
    //printk("addr(0x%x) = 0x%x\n", REG_KEYTABLE_OPERATION, HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_OPERATION)));
    //printk("addr(0x%x) = 0x%x\n", REG_KEYTABLE_WRITE_DATA0, HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_WRITE_DATA0)));
    //printk("addr(0x%x) = 0x%x\n", REG_KEYTABLE_WRITE_DATA1, HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_WRITE_DATA1)));
    //printk("addr(0x%x) = 0x%x\n", REG_KEYTABLE_WRITE_DATA2, HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_WRITE_DATA2)));
    //printk("addr(0x%x) = 0x%x\n", REG_KEYTABLE_WRITE_DATA3, HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_WRITE_DATA3)));
    //printk("addr(0x%x) = 0x%x\n", 0xBF304004, 1);
    //trigger a operation
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_START), 0x01);

    //wait bit0 clear	????????????????????????????????
    while (HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_START)) & 1)
	;
}

void hw_kt_read_key(HW_KT_SLOT_ID_E slot_id, u8 *p_key, HW_KT_SLOT_SIZE_E size)
{
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_KEY_DEBUG), 1);

    //wait bit0 clear
    while (HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_START)) & 1)
	;

    //config little endian
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_ENDIAN_CONFIG), 0x0);
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_OPERATION), HW_KT_OPER_R | (slot_id << 8));

    //trigger a operation
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_START), 0x01);

    //wait bit0 clear
    while (HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_START)) & 1)
	;

    if (HW_KT_SLOT_8B_SIZE == size) //8bit
    {
	memcpy_from_reg(p_key, (u8 *)REG_KEYTABLE_KEY_DATA2, 8);
    } else //16bit
    {
	memcpy_from_reg(p_key, (u8 *)REG_KEYTABLE_KEY_DATA0, 16);
    }
    //u32 *key = (u32 *)p_key;
    //key[0] = HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_KEY_DATA0));
    //key[1] = HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_KEY_DATA1));
    //key[2] = HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_KEY_DATA2));
    //key[3] = HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_KEY_DATA3));
}

void hw_kt_write_iv(HW_KT_SLOT_ID_E slot_id, u8 *p_iv, HW_KT_SLOT_SIZE_E size)
{
    //wait bit0 clear
    while (HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_START)) & 1)
	;

    //config little endian
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_ENDIAN_CONFIG), 0x0);
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_OPERATION), HW_KT_OPER_W_IV | (slot_id << 8));

    if (HW_KT_SLOT_8B_SIZE == size) //8bit
    {
	memcpy_to_reg((u8 *)REG_KEYTABLE_WRITE_DATA2, p_iv, 8);
    } else //16bit
    {
	memcpy_to_reg((u8 *)REG_KEYTABLE_WRITE_DATA0, p_iv, 16);
    }

    //trigger a operation
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_START), 0x01);

    //wait bit0 clear
    while (HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_START)) & 1)
	;
}

void hw_kt_read_iv(HW_KT_SLOT_ID_E slot_id, u8 *p_iv, HW_KT_SLOT_SIZE_E size)
{
    //wait bit0 clear
    while (HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_START)) & 1)
	;

    //config little endian
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_ENDIAN_CONFIG), 0x0);
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_OPERATION), HW_KT_OPER_R | (slot_id << 8));

    //trigger a operation
    HAL_PUT_U32((volatile u32 *)(REG_KEYTABLE_START), 0x01);

    //wait bit0 clear	?????????????????????????????????????
    while (HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_START)) & 1)
	;

    if (HW_KT_SLOT_8B_SIZE == size) //8bit
    {
	memcpy_from_reg(p_iv, (u8 *)REG_KEYTABLE_READ_DATA2, 8);
    } else //16bit
    {
	memcpy_from_reg(p_iv, (u8 *)REG_KEYTABLE_READ_DATA0, 16);
    }
    //u32 *iv = (u32 *)p_iv;
    //iv[0] = HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_READ_DATA0));
    //iv[1] = HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_READ_DATA1));
    //iv[2] = HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_READ_DATA2));
    //iv[3] = HAL_GET_U32((volatile u32 *)(REG_KEYTABLE_READ_DATA3));
}

