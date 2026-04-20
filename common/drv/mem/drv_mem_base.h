#ifndef __COMMON_MEM_DRV_PRIVATE_H__
#define __COMMON_MEM_DRV_PRIVATE_H__

#ifdef __cplusplus
extern "C" {
#endif

extern mt_u8* kmem_file_name;
extern mt_u32 kmem_line_number;

#define KMEM_FILE_LINE kmem_file_name = __FILE__, kmem_line_number = __LINE__

mt_void*    mt_kmalloc(mt_u32 module_id, mt_u32 size, mt_s32 flags);
mt_void     mt_kfree(mt_u32 module_id, mt_void *ptr);

mt_void*    mt_vmalloc(mt_u32 module_id, mt_u32 size);
mt_void     mt_vfree(mt_u32 module_id, mt_void *ptr);


#define mt_kmalloc(module_id, size, flags)    (KMEM_FILE_LINE, mt_kmalloc(module_id, size, flags))
#define mt_kfree(  module_id,  addr)          (KMEM_FILE_LINE, mt_kfree(  module_id, addr))
#define mt_vmalloc(module_id, size)           (KMEM_FILE_LINE, mt_vmalloc(module_id, size))
#define mt_vfree(  module_id, addr)           (KMEM_FILE_LINE, mt_vfree(  module_id, addr))

#ifdef __cplusplus
}
#endif

#endif
