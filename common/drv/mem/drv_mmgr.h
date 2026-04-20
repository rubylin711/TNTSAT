
/** @{ */
#ifndef __KMODULE_MEM_MGR_H__
#define __KMODULE_MEM_MGR_H__

#ifdef __cplusplus
extern "C"
{
#endif

struct moduleCount_moduleMemCount {
	mt_u32 u32ModuleCount;
	mt_u32 u32ModuleMemCount;
};

mt_s32 kmodule_memmgr_init(mt_u32 u32ModuleCount, mt_u32 u32ModuleMemCount);
mt_void kmodule_memmgr_exit(mt_void);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
