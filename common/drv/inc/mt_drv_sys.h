#ifndef  __MT_DRV_SYS_H__
#define  __MT_DRV_SYS_H__

#include "mt_type.h"
#include "mt_debug.h"
#include "mt_common.h"

/* Define Debug Level For SYS */
#define MT_FATAL_SYS(fmt...) MT_FATAL_PRINT(MT_ID_SYS, fmt)
#define MT_ERR_SYS(  fmt...) MT_ERR_PRINT(MT_ID_SYS, fmt)
#define MT_WARN_SYS( fmt...) MT_WARN_PRINT(MT_ID_SYS, fmt)
#define MT_INFO_SYS( fmt...) MT_INFO_PRINT(MT_ID_SYS, fmt)

typedef enum
{
    MT_CHIP_PACKAGE_TYPE_BGA_15_15 = 0,
    MT_CHIP_PACKAGE_TYPE_BGA_19_19,
    MT_CHIP_PACKAGE_TYPE_BGA_23_23,
    MT_CHIP_PACKAGE_TYPE_QFP_216,
    MT_CHIP_PACKAGE_TYPE_BUTT
} MT_CHIP_PACKAGE_TYPE_E;

extern mt_s32  mt_drv_sys_init(mt_void);
extern mt_void mt_drv_sys_exit(mt_void);

extern mt_s32  mt_drv_sys_kinit(mt_void);
extern mt_void mt_drv_sys_kexit(mt_void);

extern mt_s32 mt_drv_sys_getchipversion(MT_CHIP_TYPE_E *penChipType, MT_CHIP_VERSION_E *penChipVersion);
extern mt_s32 mt_drv_sys_getchippackagetype(MT_CHIP_PACKAGE_TYPE_E *penPackageType);

extern mt_s32 mt_drv_sys_gettimestampms(mt_u32 *pu32TimeMs);
extern mt_s32 mt_drv_sys_getdolbysupport(mt_u32 *pu32Support);
extern mt_s32 mt_drv_sys_getdtssupport(mt_u32 *pu32Support);
extern mt_s32 mt_drv_sys_getadvcasupport(mt_u32 * pu32Support);
extern mt_s32 mt_drv_sys_getrovisupport(mt_u32 *pu32Support);
extern mt_s32 mt_drv_sys_getmemconfig(mt_sys_mem_config_s *pstConfig);

#endif /* __MT_DRV_SYS_H__ */

