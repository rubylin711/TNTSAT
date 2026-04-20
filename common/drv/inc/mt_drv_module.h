
#ifndef __MT_DRV_MODULE_H__
#define __MT_DRV_MODULE_H__

#include "mt_type.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

#define MT_KMODULE_MAX_COUNT      (256)
#define MT_KMODULE_MEM_MAX_COUNT  (256*256)

#define MT_FATAL_MODULE(fmt...)    MT_FATAL_PRINT(MT_ID_MODULE, fmt)
#define MT_ERR_MODULE(fmt...)      MT_ERR_PRINT(MT_ID_MODULE, fmt)
#define MT_WARN_MODULE(fmt...)     MT_WARN_PRINT(MT_ID_MODULE, fmt)
#define MT_INFO_MODULE(fmt...)     MT_INFO_PRINT(MT_ID_MODULE, fmt)

mt_s32  mt_drv_mmngr_init(mt_u32 u32ModuleCount, mt_u32 u32ModuleMemCount);
mt_void mt_drv_mmngr_exit(mt_void);

mt_s32 mt_drv_module_allocid(mt_u8* pu8ModuleName, mt_u32 *pu32ModuleID, mt_s32 *ps32Status);
mt_s32 mt_drv_module_register(mt_u32 u32ModuleID, const mt_u8* pu8ModuleName, mt_void* pFunc);
mt_s32 mt_drv_module_unregister(mt_u32 u32ModuleID);

#ifdef CMN_MMGR_SUPPORT
mt_u8* mt_drv_module_getname_byid(mt_u32 u32ModuleID);
mt_u32 mt_drv_module_getid_byname(mt_u8* pu8Name);
#endif

mt_s32 mt_drv_module_getfunction(mt_u32 u32ModuleID, mt_void** ppFunc);

mt_s32  mmngr_drv_modinit(mt_u32 u32ModuleCount, mt_u32 u32ModuleMemCount);
mt_void mmngr_drv_modexit(mt_void);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MT_DRV_MODULE_H__ */

