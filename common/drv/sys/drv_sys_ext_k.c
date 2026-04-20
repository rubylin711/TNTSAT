/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/kernel.h>
#include <linux/module.h>
//#include <mach/hardware.h>
#include <linux/sched.h>
#include <linux/math64.h>

#include "mt_type.h"
#include "mt_debug.h"
#include "mt_reg_common.h"
#include "mt_drv_sys.h"
#include "mt_drv_reg.h"
#include <uapi/linux/mtuapi.h>
#include "mt_mach/chipinfo.h"

#define DIV_NS_TO_MS  1000000

mt_s32 mt_drv_sys_getchipversion(MT_CHIP_TYPE_E *penChipType, MT_CHIP_VERSION_E *penChipVersion)
{
#ifdef CONFIG_MT_CHIP_ARIA
    MT_CHIP_VERSION_E enChipVersion = MT_CHIP_VERSION_BUTT;
#endif

    /* penChipType or penChipVersion maybe NULL, but not both */
    if (MT_NULL == penChipType || MT_NULL == penChipVersion)
    {
        MT_ERR_SYS("invalid input parameter 000\n");
        return MT_FAILURE;
    }

    if (*penChipType == MT_CHIP_TYPE_BUTT)
    {
        MT_ERR_SYS("invalid input parameter 111\n");
        return MT_FAILURE;
    }

#ifdef CONFIG_MT_CHIP_ARIA
    *penChipVersion = enChipVersion;
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    switch (symphony_get_chip_rev()) {
    case CHIP_CONCERTO_A0:
        *penChipVersion = MT_CHIP_CONCERTO_A0;
        break;
    case CHIP_CONCERTO_A1:
        *penChipVersion = MT_CHIP_CONCERTO_A1;
        break;
    case CHIP_CONCERTO_A3:
        *penChipVersion = MT_CHIP_CONCERTO_A3;
        break;
    case CHIP_CONCERTO_B0:
        *penChipVersion = MT_CHIP_CONCERTO_B0;
        break;
    case CHIP_SYMPHONY_A0:
        *penChipVersion = MT_CHIP_SYMPHONY_A0;
        break;
    case CHIP_SYMPHONY_A1:
        *penChipVersion = MT_CHIP_SYMPHONY_A1;
        break;
    case CHIP_SYMPHONY_A2:
        *penChipVersion = MT_CHIP_SYMPHONY_A2;
        break;
    case CHIP_SYMPHONY3_A0:
        *penChipVersion = MT_CHIP_SYMPHONY3_A0;
        break;
    case CHIP_SYMPHONY2_A0:
        *penChipVersion = MT_CHIP_SYMPHONY2_A0;
        break;
    case CHIP_SYMPHONY2_A1:
        *penChipVersion = MT_CHIP_SYMPHONY2_A1;
        break;
    case CHIP_SYMPHONY2_A2:
        *penChipVersion = MT_CHIP_SYMPHONY2_A2;
        break;
    case CHIP_SYMPHONY2_A3:
        *penChipVersion = MT_CHIP_SYMPHONY2_A3;
        break;
    default:
        *penChipVersion = MT_CHIP_VERSION_BUTT;
    }
#elif defined (CONFIG_MT_CHIP_SYMPHONY4)
    switch (symphony_get_chip_rev())
    {
	case CHIP_SYMPHONY4_A0:
	*penChipVersion = MT_CHIP_SYMPHONY4_A0;
        break;
    case CHIP_SYMPHONY4_A1:
        *penChipVersion = MT_CHIP_SYMPHONY4_A1;
        break;
    default:
        *penChipVersion = MT_CHIP_VERSION_BUTT;
    }
#elif defined (CONFIG_MT_CHIP_SYMPHONY6)
    switch (symphony_get_chip_rev())
    {
	case CHIP_SYMPHONY6_A0:
		*penChipVersion = MT_CHIP_SYMPHONY6_A0;
        break;
	case CHIP_SYMPHONY6_A1:
		*penChipVersion = MT_CHIP_SYMPHONY6_A1;
		break;
    default:
        *penChipVersion = MT_CHIP_VERSION_BUTT;
    }
#endif

    return MT_SUCCESS;
}

mt_s32 mt_drv_sys_getchippackagetype(MT_CHIP_PACKAGE_TYPE_E *penPackageType)
{
    if (MT_NULL == penPackageType)
    {
        MT_ERR_SYS("invalid input parameter\n");
        return MT_FAILURE;
    }
    *penPackageType = MT_CHIP_PACKAGE_TYPE_BGA_23_23;
     return MT_SUCCESS;
}

mt_s32 mt_drv_sys_gettimestampms(mt_u32 *pu32TimeMs)
{
    if (MT_NULL == pu32TimeMs)
    {
        MT_ERR_SYS("null pointer error\n");
        return MT_FAILURE;
    }

    *pu32TimeMs = (mt_u32)(jiffies - INITIAL_JIFFIES) * (MSEC_PER_SEC / HZ);


    return MT_SUCCESS;
}

/*
 * from datasheet, the value of dolby_flag meaning: 0: support; 1: not_support .
 * but we change its meaning for return parameter : 0:not support; 1:support.
 */
mt_s32 mt_drv_sys_getdolbysupport(mt_u32 *pu32Support)
{
    *pu32Support = 0;
    return MT_SUCCESS;
}

/*
 * 1:support; 0:not_support
 */
mt_s32 mt_drv_sys_getdtssupport(mt_u32 *pu32Support)
{
    *pu32Support = 0;
    return MT_SUCCESS;
}

/*
 * 1:support; 0:not_support
 */
mt_s32 mt_drv_sys_getadvcasupport(mt_u32 *pu32Support)
{
    *pu32Support = 0;
    return MT_SUCCESS;
}

/*
 * 0: not_advca_support; 1: advca_support
 */
mt_s32 mt_drv_sys_getrovisupport(mt_u32 *pu32Support)
{
    *pu32Support = 0;
    return MT_SUCCESS;
}

mt_s32 mt_drv_sys_getmemconfig(mt_sys_mem_config_s *pstConfig)
{
#ifdef CONFIG_MT_CHIP_COMMON
    mt_u32 Ret;

    if (MT_NULL != pstConfig)
    {
        Ret = get_mem_size(&(pstConfig->u32TotalSize), MTUAPI_GET_RAM_SIZE);

        Ret |= get_mem_size(&(pstConfig->u32MMZSize), MTUAPI_GET_MMZ_SIZE);

        if (Ret != MT_SUCCESS)
        {
            MT_ERR_SYS("get_mem_size ERR, Ret=%#x\n", Ret);
            return MT_FAILURE;
        }
    }
    else
#endif
    {
        MT_ERR_SYS("invalid DDR conf ptr\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;

}

mt_s32 mt_drv_sys_kinit(mt_void)
{
    return 0;
}

mt_void mt_drv_sys_kexit(mt_void)
{
    return ;
}

EXPORT_SYMBOL(mt_drv_sys_getchipversion);
EXPORT_SYMBOL(mt_drv_sys_getchippackagetype);
EXPORT_SYMBOL(mt_drv_sys_gettimestampms);
EXPORT_SYMBOL(mt_drv_sys_getdolbysupport);
EXPORT_SYMBOL(mt_drv_sys_getdtssupport);
EXPORT_SYMBOL(mt_drv_sys_getadvcasupport);
EXPORT_SYMBOL(mt_drv_sys_getrovisupport);
EXPORT_SYMBOL(mt_drv_sys_getmemconfig);


