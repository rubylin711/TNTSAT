#ifndef __DRV_MEM_IOCTL_H__
#define __DRV_MEM_IOCTL_H__

#include "../../inc/mt_type.h"
#include "mpi_mmz.h"
#include "../../inc/mt_debug.h"
#include "mt_drv_mem.h"

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

#ifndef __KERNEL_
struct mtMEM_LLI_S
{
    mt_mmz_buf_s  buf;
    struct mtMEM_LLI_S  *next;
};

#else
struct mtMEM_LLI_S
{
    mt_mmz_buf_s  buf;
    atomic_t    cnt;
    struct mtMEM_LLI_S  *next;
};
#endif

typedef struct mtMEM_LLI_S MEM_LLI_S;

#define UMAPC_MEM_MALLOC                _IOWR(MT_ID_MEM,101, mt_mmz_buf_s)
#define UMAPC_MEM_FREE                  _IOW (MT_ID_MEM,102, mt_mmz_buf_s)

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __DRV_MEM_IOCTL_H__ */


