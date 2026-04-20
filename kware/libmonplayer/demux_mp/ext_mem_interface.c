#include "mt_type.h"
#include "sys_define.h"

#include "mtos_mem.h"
#include "mtos_sem.h"

#include "ext_mem_interface.h"
#include "lib_memp.h"

typedef struct
{
    unsigned int lock;
    void *p_ext_heap_hdl;
    MT_BOOL use_ext_heap;
}mem_ctrl_from_demux_mp;

mem_ctrl_from_demux_mp g_mem_ctrl_from_demux_mp = {0};
mem_ctrl_from_demux_mp *gp_mem_ctrl_from_demux_mp = &g_mem_ctrl_from_demux_mp;
inline void mem_init_from_demux_mp(unsigned char * p_ext_buf_addr, unsigned int ext_buf_size)
{
    MT_BOOL ret;
    
    if (p_ext_buf_addr != NULL && ext_buf_size != 0) {
        gp_mem_ctrl_from_demux_mp->lock = 0;
        gp_mem_ctrl_from_demux_mp->p_ext_heap_hdl = 0;
        gp_mem_ctrl_from_demux_mp->use_ext_heap = FALSE;
        gp_mem_ctrl_from_demux_mp->p_ext_heap_hdl = (lib_memp_t *)mtos_malloc(sizeof(lib_memp_t));
        
        memset(gp_mem_ctrl_from_demux_mp->p_ext_heap_hdl, 0, sizeof(lib_memp_t));
        
        ret = mtos_sem_create(&(gp_mem_ctrl_from_demux_mp->lock), TRUE);
        MT_ASSERT(ret == TRUE);

        mtos_sem_take((os_sem_t *)(&(gp_mem_ctrl_from_demux_mp->lock)), 0);
        if (lib_memp_create(gp_mem_ctrl_from_demux_mp->p_ext_heap_hdl, p_ext_buf_addr, ext_buf_size) != SUCCESS) {
            MT_ASSERT(0);
        }
        mtos_sem_give((os_sem_t *)(&(gp_mem_ctrl_from_demux_mp->lock)));
        
        gp_mem_ctrl_from_demux_mp->use_ext_heap = TRUE;
    } else {
        gp_mem_ctrl_from_demux_mp->use_ext_heap = FALSE;
    }
}

inline void * calloc_from_demux_mp(unsigned int n, unsigned int size)
{
    void * p_buf = NULL;

    if (gp_mem_ctrl_from_demux_mp->use_ext_heap) {
        mtos_sem_take((os_sem_t *)(&(gp_mem_ctrl_from_demux_mp->lock)), 0);
        p_buf = lib_memp_alloc(gp_mem_ctrl_from_demux_mp->p_ext_heap_hdl, n*size);
        mtos_sem_give((os_sem_t *)(&(gp_mem_ctrl_from_demux_mp->lock)));
   } else {
        p_buf = mtos_malloc(n*size);
    }

    if (p_buf) {
        memset(p_buf, 0x00, n * size);
    }
    
    return p_buf;
}

inline void *malloc_from_demux_mp(unsigned int size)
{
    void * p_buf = NULL;

    if (gp_mem_ctrl_from_demux_mp->use_ext_heap) {
        mtos_sem_take((os_sem_t *)(&(gp_mem_ctrl_from_demux_mp->lock)), 0);
        p_buf = lib_memp_alloc(gp_mem_ctrl_from_demux_mp->p_ext_heap_hdl, size);
        mtos_sem_give((os_sem_t *)(&(gp_mem_ctrl_from_demux_mp->lock)));
    } else {
        p_buf = mtos_malloc(size);
    }

    return p_buf;
}

inline void * realloc_from_demux_mp(void * old_ptr, unsigned int size)
{
    void * new_ptr = NULL;

    new_ptr = malloc_from_demux_mp(size);
    
    if (new_ptr == NULL) {
        OS_PRINTF("%s: malloc memory failed(size:%d\n", __func__, size);
        return new_ptr;
    }

     if (old_ptr) {
         memcpy(new_ptr, old_ptr, size);
         free_from_demux_mp(old_ptr);
    }

     return new_ptr;
}

inline void free_from_demux_mp(void * ptr)
{
    RET_CODE ret = SUCCESS;

    if (gp_mem_ctrl_from_demux_mp->use_ext_heap) {
        mtos_sem_take((os_sem_t *)(&(gp_mem_ctrl_from_demux_mp->lock)), 0);
        ret = lib_memp_free(gp_mem_ctrl_from_demux_mp->p_ext_heap_hdl, ptr);
        MT_ASSERT(ret == SUCCESS);
        mtos_sem_give((os_sem_t *)(&(gp_mem_ctrl_from_demux_mp->lock)));
    } else {
        mtos_free(ptr);
    }
}

inline void mem_release_from_demux_mp()
{
    if (gp_mem_ctrl_from_demux_mp->use_ext_heap) {
        lib_memp_destroy(gp_mem_ctrl_from_demux_mp->p_ext_heap_hdl);
        mtos_sem_destroy(&(gp_mem_ctrl_from_demux_mp->lock), 0);
    }
    
    gp_mem_ctrl_from_demux_mp->use_ext_heap = FALSE;
}


