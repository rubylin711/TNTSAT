
/** @{ */
#ifndef __KMODULE_MEM_H__
#define __KMODULE_MEM_H__

#ifdef __cplusplus
extern "C"{
#endif

typedef enum tagKMemType
{
    KMEM_TYPE_MMZ,
    KMEM_TYPE_KMEM,
    KMEM_TYPE_VMEM,

    KMEM_TYPE_BUTT
}KMEM_TYPE_E;

typedef mt_s32 (* fnKMemStatCallback)(mt_u32 u32ModuleID, KMEM_TYPE_E enMemType, mt_s32 s32Size);


mt_s32 kmodule_mem_pool_del_module(mt_u32 u32ModuleID);
mt_s32 kmodule_mem_pool_add_module(mt_u32 u32ModuleID);


mt_s32 kmodule_mem_init(mt_u32 u32CountMem, fnKMemStatCallback fnCallback);

mt_s32 kmodule_mem_deinit(mt_void);


#ifdef __cplusplus
}
#endif

#endif
