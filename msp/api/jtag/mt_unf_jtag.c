/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mt_type.h"
#include "mt_common.h"

#include "mt_unf_misc.h"
#include "mt_unf_gpio.h"
#include "mt_unf_jtag.h"

#include "drv_sys_misc.h"

#define REG_DEBUG_KEY_CONFIG 0x1F30D010

#define REG_SW_PINSEL2 0x1F13C008
#define REG_SW_PINSEL5 0x1F13C014
#define REG_SW_PINSEL6 0x1F13C018

/** TODO: TO distinguish S1/S2 */
static mt_s32 jtag_set_pinmux(void)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 reg_data = 0;

    mt_sys_version_s stSysChipInfo;

    memset(&stSysChipInfo, 0, sizeof(mt_sys_version_s));

    ret = mt_sys_get_version(&stSysChipInfo);
    if (ret != MT_SUCCESS) {
	return MT_FAILURE;
    }

    if (MT_CHIP_SYMPHONY2_A0 <= stSysChipInfo.enChipVersion && MT_CHIP_SYMPHONY2_A3 >= stSysChipInfo.enChipVersion) {
	//pinsel6
	ret = mt_sys_read_register(REG_SW_PINSEL6, &reg_data);
	reg_data &= ~(mt_u32)0xf;
	reg_data |= (mt_u32)0x2;
	ret |= mt_sys_write_register(REG_SW_PINSEL6, reg_data);

	//pinsel2
	ret = mt_sys_read_register(REG_SW_PINSEL2, &reg_data);
	reg_data &= ~((mt_u32)0xfff << 16);
	reg_data |= ((mt_u32)0x333 << 16);
	ret |= mt_sys_write_register(REG_SW_PINSEL2, reg_data);

    } else if (MT_CHIP_SYMPHONY_A0 == stSysChipInfo.enChipVersion || MT_CHIP_SYMPHONY_A1 == stSysChipInfo.enChipVersion || MT_CHIP_SYMPHONY_A2 == stSysChipInfo.enChipVersion) {
	//pinsel2
	ret = mt_sys_read_register(REG_SW_PINSEL2, &reg_data);
	reg_data &= ~((mt_u32)0xffff << 12);
	reg_data |= ((mt_u32)0x3333 << 12);
	ret |= mt_sys_write_register(REG_SW_PINSEL2, reg_data);
    } else {
	//Not implemented
	return MT_FAILURE;
    }

    ret |= mt_sys_write_register(0x1f1200c0, 0);
    return ret;
}

/** TODO: TO distinguish S1/S2 */
static mt_s32 jtag_set_password(mt_u8 *p_password, mt_u32 len)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 reg_data = 0;
    mt_u8 *password = NULL;

    if (p_password == NULL) {
	return MT_FAILURE;
    }

    if ((len != 8) && (len != 16)) {
	//bad parameter
	return MT_FAILURE;
    }

    password = p_password;

    reg_data = (mt_u32)password[0] | (mt_u32)password[1] << 8 | (mt_u32)password[2] << 16 | (mt_u32)password[3] << 24;
    ret = mt_sys_write_register(REG_DEBUG_KEY_CONFIG, reg_data);

    reg_data = (mt_u32)password[4] | (mt_u32)password[5] << 8 | (mt_u32)password[6] << 16 | (mt_u32)password[7] << 24;
    ret |= mt_sys_write_register((REG_DEBUG_KEY_CONFIG + 4), reg_data);

    return ret;
}

static mt_s32 jtag_set_master_password(mt_u8 *p_password, mt_u32 len)
{
    //TODO
    return MT_SUCCESS;
}

mt_s32 MT_UNF_JTAG_set_password(mt_u8 *p_password, mt_u32 len)
{
    mt_s32 ret = MT_SUCCESS;

    ret = jtag_set_pinmux();

    ret |= jtag_set_password(p_password, len);

    return ret;
}

mt_s32 MT_UNF_JTAG_set_master_password(mt_u8 *p_password, mt_u32 len)
{
    mt_s32 ret = MT_SUCCESS;

    ret = jtag_set_pinmux();

    ret |= jtag_set_master_password(p_password, len);

    return ret;
}

void MT_UNF_JTAG_toggle(void)
{
    jtag_set_pinmux();
}
