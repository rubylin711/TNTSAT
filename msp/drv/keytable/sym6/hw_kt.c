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

static inline mt_u32 io_read32(ulong addr)
{
    return HAL_GET_U32((volatile void *)addr);
}

static inline void io_write32(ulong addr, mt_u32 val)
{
    HAL_PUT_U32((volatile void *)addr, val);
}

static void memcpy_from_reg(unsigned char *dest, const unsigned char *reg, int len)
{
    mt_u32 reg_size = len >> 2;
    const mt_u32 *src = (const mt_u32 *)reg;
    mt_u8 *p = (mt_u8 *)dest;
    mt_u32 value = 0;

    while (reg_size--) {
        value = io_read32((ulong)src);
        src++;
        *p++ = value & 0xff;
        *p++ = (value >> 8) & 0xff;
        *p++ = (value >> 16) & 0xff;
        *p++ = (value >> 24) & 0xff;
    }
}

static void memcpy_to_reg(unsigned char *reg, const unsigned char *src, int len)
{
    mt_u32 reg_size = len >> 2;
    mt_u32 *dst = (mt_u32 *)reg;
    const mt_u8 *p = (const mt_u8 *)src;
    mt_u32 value = 0;

    while (reg_size--) {
        value = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
        io_write32((ulong)dst, value);
        dst++;
        p += 4;
    }
}

typedef union _HW_KT_KEY_ATTR_REG {
	mt_u32 all;
	struct {
		/* [13:0] key usage */
		mt_u32 AES_OFFON:1;
		mt_u32 DES_OFFON:1;
		mt_u32 TDES_OFFON:1;
		mt_u32 CSAv2_OFFON:1;
		mt_u32 CSAv3_OFFON:1;
		mt_u32 SM4_OFFON:1;
		mt_u32 CSAv2_CONFORMANCE_OFFON:1;
		mt_u32 GOST_28147_89_OR_R34_12_MAGMA_OFFON:1;
		mt_u32 ASA_OFFON:1;
		mt_u32 GOST_R34_12_KUZNYECHIK_OFFON:1;
		mt_u32 MULTI2_OFFON:1;
		mt_u32 resv1:3;

		/* [16:14] */
		mt_u32 TS_OFFON:1;
		mt_u32 M2M_OFFON:1;
		mt_u32 MAC_OFFON:1;

		/* [17] */
		mt_u32 M2M_REE_OFFON:1;

		/* [19:18] */
		mt_u32 DEC_ONOFF:1;
		mt_u32 ENC_ONOFF:1;

		/* [22:20] */
		mt_u32 KEY_SIZE64_OFFON:1;
		mt_u32 KEY_SIZE128_OFFON:1;
		mt_u32 KEY_SIZE256_OFFON:1;

		/* [27:23] */
		mt_u32 resv2:5;

		/* [31:28] */
		mt_u32 KEY_SOURCE:4;
	} bitc;
} HW_KT_KEY_ATTR_REG;

typedef union _HW_KT_TEE_REG {
	mt_u32 all;
	struct {
		mt_u32 TEE_PRM:4;
		mt_u32 TEE_WCID:5;
		mt_u32 TEE_SCID:5;
		mt_u32 TEE_SC:1;
		mt_u32 TEE_TP:1;
		mt_u32 TEE_ENC_OFFON:1;
		mt_u32 TEE_DEC_OFFON:1;
		mt_u32 TEE_AUDIO:1;
		mt_u32:13;
	} bitc;
} HW_KT_TEE_REG;

typedef union _HW_TEE_CFG_DATA_CHK_REG1 {
	mt_u32 all;
	struct {
		mt_u32 invalid_keyslot:1;
		mt_u32 chk_field_1t31:31;
	} bitc;
} HW_TEE_CFG_DATA_CHK_REG1;

typedef union _HW_TEE_CFG_DATA_CHK_REG2 {
	mt_u32 all;
	struct {
		mt_u32 chk_field_32t35:4;
		mt_u32 resv:28;
	} bitc;
} HW_TEE_CFG_DATA_CHK_REG2;

typedef union _HW_KT_K256_INFO {
	mt_u32 all;
	struct {
		mt_u32 k256_err_index:7;
		mt_u32 resv2:1;
		mt_u32 k256_err_sta:1;
		mt_u32 resv1:23;
	} bitc;
} HW_KT_K256_INFO;

void hw_kt_write_valid(HW_KT_SLOT_ID_E slot_id, HW_KT_SLOT_VALID_E valid)
{
	if (slot_id >= HW_KT_SLOT_MAX_NUM)
		return;

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	/* config big endian */
	io_write32(KT_ENDIAN,  0x01);
	io_write32(KT_OPERATION, HW_KT_OPER_VALID | (slot_id << 8));
	io_write32(KT_WR_DATA_31T0, valid);

	/* trigger a operation */
	io_write32(KT_START, 0x01);

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;
}

void hw_kt_read_valid(HW_KT_SLOT_ID_E slot_id, HW_KT_SLOT_VALID_E *p_valid)
{
	if (slot_id >= HW_KT_SLOT_MAX_NUM)
		return;

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	/* config little endian */
	io_write32(KT_ENDIAN, 0x0);
	io_write32(KT_OPERATION, HW_KT_OPER_R | (slot_id << 8));

	/* trigger a operation */
	io_write32(KT_START, 0x01);

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	*p_valid = io_read32(KT_RD_DATA_SLOTVALID) & 0x1;
}

void hw_kt_write_attribute(HW_KT_SLOT_ID_E slot_id, mt_u32 attr)
{
	if (slot_id >= HW_KT_SLOT_MAX_NUM)
		return;

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	/* config big endian */
	io_write32(KT_ENDIAN, 0x1);
	io_write32(KT_OPERATION, HW_KT_OPER_W_ATTR | (slot_id << 8));
	io_write32(KT_WR_DATA_31T0, attr);

	/* trigger a operation */
	io_write32(KT_START, 0x01);

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;
}

void hw_kt_read_attribute(HW_KT_SLOT_ID_E slot_id, mt_u32 *p_attr)
{
	if (slot_id >= HW_KT_SLOT_MAX_NUM)
		return;

	if (!p_attr)
		return;

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	/* config little endian */
	io_write32(KT_ENDIAN, 0x0);
	io_write32(KT_OPERATION, HW_KT_OPER_R | (slot_id << 8));

	/* trigger a operation */
	io_write32(KT_START, 0x01);

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	*p_attr = io_read32(KT_RD_DATA_ATTR);
}

void hw_kt_write_key(HW_KT_SLOT_ID_E slot_id, const mt_u8 *p_key,
		     HW_KT_SLOT_SIZE_E size)
{
	if (slot_id >= HW_KT_SLOT_MAX_NUM)
		return;

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	/* config little endian */
	io_write32(KT_ENDIAN, 0x0);
	io_write32(KT_OPERATION, HW_KT_OPER_W_KEY | (slot_id << 8));

	if (size == HW_KT_SLOT_8B_SIZE) {
		io_write32(KT_WR_DATA_127T96, 0x0);
		io_write32(KT_WR_DATA_95T64, 0x0);
		memcpy_to_reg((mt_u8 *)KT_WR_DATA_63T32, p_key, size);
	}
	else
		memcpy_to_reg((mt_u8 *)KT_WR_DATA_127T96, p_key, size);

	/* trigger a operation */
	io_write32(KT_START, 0x01);

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;
}

void hw_kt_read_key(HW_KT_SLOT_ID_E slot_id, mt_u8 *p_key,
		    HW_KT_SLOT_SIZE_E size)
{
	if (slot_id >= HW_KT_SLOT_MAX_NUM)
		return;

	io_write32(KT_RD_KEY_DEBUG, 1);

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	/* config little endian */
	io_write32(KT_ENDIAN, 0x0);
	io_write32(KT_OPERATION, HW_KT_OPER_R | (slot_id << 8));

	/* trigger a operation */
	 io_write32(KT_START, 0x01);

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	if (size == HW_KT_SLOT_8B_SIZE)
		memcpy_from_reg(p_key, (mt_u8 *)KT_RD_KEY_63T32, size);
	else
		memcpy_from_reg(p_key, (mt_u8 *)KT_RD_KEY_127T96, size);
}

void hw_kt_write_iv(HW_KT_SLOT_ID_E slot_id, const mt_u8 *p_iv,
		    HW_KT_SLOT_SIZE_E size)
{
	if (slot_id >= HW_KT_SLOT_MAX_NUM)
		return;

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	/* config little endian */
	io_write32(KT_ENDIAN, 0x0);
	io_write32(KT_OPERATION, HW_KT_OPER_W_IV | (slot_id << 8));

	if (size == HW_KT_SLOT_8B_SIZE) {
		io_write32(KT_WR_DATA_127T96, 0x0);
		io_write32(KT_WR_DATA_95T64, 0x0);
		memcpy_to_reg((mt_u8 *)KT_WR_DATA_63T32, p_iv, size);
	}
	else
		memcpy_to_reg((mt_u8 *)KT_WR_DATA_127T96, p_iv, size);

	/* trigger a operation */
	io_write32(KT_START, 0x01);

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;
}

void hw_kt_read_iv(HW_KT_SLOT_ID_E slot_id, mt_u8 *p_iv,
		   HW_KT_SLOT_SIZE_E size)
{
	if (slot_id >= HW_KT_SLOT_MAX_NUM)
		return;

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	/* config little endian */
	io_write32(KT_ENDIAN, 0x0);
	io_write32(KT_OPERATION, HW_KT_OPER_R | (slot_id << 8));

	/* trigger a operation */
	io_write32(KT_START, 0x01);

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	if (size == HW_KT_SLOT_8B_SIZE)
		memcpy_from_reg(p_iv, (mt_u8 *)KT_RD_DATA_63T32, size);
	else
		memcpy_from_reg(p_iv, (mt_u8 *)KT_RD_DATA_127T96, size);
}

void hw_kt_write_key256(HW_KT_SLOT_ID_E slot_id, const mt_u8 *p_key)
{
	if (slot_id >= HW_KT_SLOT_MAX_NUM)
		return;

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	/* config little endian */
	io_write32(KT_ENDIAN, 0x0);
	io_write32(KT_OPERATION, HW_KT_OPER_W_KEY256 | (slot_id << 8));

	memcpy_to_reg((mt_u8 *)KT_WR_DATA_255T224, &p_key[0], HW_KT_SLOT_16B_SIZE);
	memcpy_to_reg((mt_u8 *)KT_WR_DATA_127T96, &p_key[16], HW_KT_SLOT_16B_SIZE);

	/* trigger a operation */
	io_write32(KT_START, 0x01);

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;
}

void hw_kt_set_k256_endian(mt_u32 k256_endian)
{
        mt_u32 data;

	data = io_read32(KT_K256_INFO);
	data &= ~(1 << 16);
	data |= (k256_endian & 0x1) << 16;

	io_write32(KT_K256_INFO, data);
}

void hw_kt_get_k256_info(mt_u32 *k256_err_sta, mt_u32 *k256_err_index)
{
	HW_KT_K256_INFO k256_info;

	k256_info.all = io_read32(KT_K256_INFO);

	*k256_err_sta = k256_info.bitc.k256_err_sta;
	*k256_err_index = k256_info.bitc.k256_err_index;
}

void hw_kt_clear_k256_sta(void)
{
        mt_u32 data;

	data = io_read32(KT_K256_INFO);
	data |= 1 << 12;
	io_write32(KT_K256_INFO, data);
}

void hw_kt_nonce_clear(void)
{
	io_write32(KT_RNG_CLR, 0x1);
}

void hw_kt_get_nonce_status(HW_KT_NONCE_STATUS_E *p_stat)
{
	*p_stat = (io_read32(KT_RNG_STATUS) >> 4) & 0x1;
}

void hw_kt_write_tee(HW_KT_SLOT_ID_E slot_id, mt_u32 tee_cfg)
{
	if (slot_id >= HW_KT_SLOT_MAX_NUM)
		return;

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	/* config big endian */
	io_write32(KT_ENDIAN, 0x01);
	io_write32(KT_OPERATION, HW_KT_OPER_W_TEE | (slot_id << 8));
	io_write32(KT_WR_DATA_31T0, tee_cfg);

	/* trigger a operation */
	io_write32(KT_START, 0x01);

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;
}

void hw_kt_read_tee(HW_KT_SLOT_ID_E slot_id, mt_u32 *p_tee)
{
	if (slot_id >= HW_KT_SLOT_MAX_NUM)
		return;

	if (!p_tee)
		return;

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1) ;

	/* config little endian */
	io_write32(KT_ENDIAN, 0x0);
	io_write32(KT_OPERATION, HW_KT_OPER_R_TEE | (slot_id << 8));

	/* trigger a operation */
	io_write32(KT_START, 0x01);

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1) ;

	*p_tee = io_read32(KT_RD_DATA_TEE);
}

void hw_kt_read_tee_valid(
		HW_KT_SLOT_ID_E slot_id,
		HW_KT_SLOT_TEE_VALID_E *p_valid)
{
	if (slot_id >= HW_KT_SLOT_MAX_NUM)
		return;

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	/* config little endian */
	io_write32(KT_ENDIAN, 0x0);
	io_write32(KT_OPERATION, HW_KT_OPER_R | (slot_id << 8));

	/* trigger a operation */
	io_write32(KT_START, 0x01);

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	*p_valid = (io_read32(KT_RD_DATA_SLOTVALID) >> 31) & 0x1;
}

void hw_kt_write_tee_cfgdata_chk_lock(HW_TEE_CFG_DATA_CHK_LOCK_E lock)
{
	mt_u32 data;

	data = io_read32(KT_WR_TEE_CFG_DATA_CHK_LOCK);
	data &= ~(1 << 0);
	data |= (lock & 0x1) << 0;
	io_write32(KT_WR_TEE_CFG_DATA_CHK_LOCK, data);
}

void hw_kt_read_tee_cfgdata_chk_lock(HW_TEE_CFG_DATA_CHK_LOCK_E *p_lock)
{
	*p_lock = (io_read32(KT_WR_TEE_CFG_DATA_CHK_LOCK) >> 0) & 1;
}

void hw_kt_write_tee_cfgdata_checken(HW_TEE_CFG_DATA_CHKEN_E enable)
{
	mt_u32 data;

	data = io_read32(KT_MODE);
	data &= ~(1 << 4);
	data |= (enable & 0x1) << 4;
	io_write32(KT_MODE, data);
}

void hw_kt_read_tee_cfgdata_checken(HW_TEE_CFG_DATA_CHKEN_E *p_enable)
{
	*p_enable = (io_read32(KT_MODE) >> 4) & 1;
}

void hw_kt_write_tee_cfgdata_chk(HW_TEE_CFG_DATA_CHK_S *p_check)
{
	HW_TEE_CFG_DATA_CHK_REG1 reg1 = { 0 };
	HW_TEE_CFG_DATA_CHK_REG2 reg2 = { 0 };

	reg1.bitc.invalid_keyslot = p_check->invalid_keyslot;
	reg1.bitc.chk_field_1t31 = p_check->chk_field_1t31;
	reg2.bitc.chk_field_32t35 = p_check->chk_field_32t35;

	io_write32(KT_WR_TEE_CFG_DATA_CHK_31t0, reg1.all);
	io_write32(KT_WR_TEE_CFG_DATA_CHK_35t32, reg2.all);
}

void hw_kt_read_tee_cfgdata_chk(HW_TEE_CFG_DATA_CHK_S *p_check)
{
	HW_TEE_CFG_DATA_CHK_REG1 reg1 = { 0 };
	HW_TEE_CFG_DATA_CHK_REG2 reg2 = { 0 };

	reg1.all = io_read32(KT_WR_TEE_CFG_DATA_CHK_31t0);
	reg2.all = io_read32(KT_WR_TEE_CFG_DATA_CHK_35t32);
	p_check->invalid_keyslot = reg1.bitc.invalid_keyslot;
	p_check->chk_field_1t31 = reg1.bitc.chk_field_1t31;
	p_check->chk_field_32t35 = reg2.bitc.chk_field_32t35;
}

void hw_kt_read_metadata(HW_KT_SLOT_ID_E slot_id, mt_u32 *p_metadata)
{
	if (slot_id >= HW_KT_SLOT_MAX_NUM)
		return;

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	/* config little endian */
	io_write32(KT_ENDIAN, 0x0);
	io_write32(KT_OPERATION, HW_KT_OPER_R | (slot_id << 8));

	/* trigger a operation */
	io_write32(KT_START, 0x01);

	/* wait bit0 clear */
	while (io_read32(KT_START) & 1)
		;

	*p_metadata = io_read32(KT_RD_DATA_METADATA);
}

void hw_kt_read_sw_reg(uint32_t *p_reg, uint32_t index)
{
	mt_u64 addr;

	switch (index) {
	case 0:
		addr = KT_SLOT_SW_15T0;
		break;
	case 1:
		addr = KT_SLOT_SW_31T16;
		break;
	case 2:
		addr = KT_SLOT_SW_47T32;
		break;
	case 3:
		addr = KT_SLOT_SW_63T48;
		break;
	case 4:
		addr = KT_SLOT_SW_79T64;
		break;
	case 5:
		addr = KT_SLOT_SW_95T80;
		break;
	case 6:
		addr = KT_SLOT_SW_111T96;
		break;
	case 7:
		addr = KT_SLOT_SW_127T112;
		break;
	default:
		return;
	}

	*p_reg = io_read32(addr);
}

void hw_kt_write_sw_reg(uint32_t data, uint32_t index)
{
	mt_u64 addr;

	switch (index) {
	case 0:
		addr = KT_SLOT_SW_15T0;
		break;
	case 1:
		addr = KT_SLOT_SW_31T16;
		break;
	case 2:
		addr = KT_SLOT_SW_47T32;
		break;
	case 3:
		addr = KT_SLOT_SW_63T48;
		break;
	case 4:
		addr = KT_SLOT_SW_79T64;
		break;
	case 5:
		addr = KT_SLOT_SW_95T80;
		break;
	case 6:
		addr = KT_SLOT_SW_111T96;
		break;
	case 7:
		addr = KT_SLOT_SW_127T112;
		break;
	default:
		return;
	}

	io_write32(addr, data);
}

