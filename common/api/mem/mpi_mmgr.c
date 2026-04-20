#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

#include "mt_type.h"
#include "mt_debug.h"
#include "mt_drv_mem.h"
#include "mt_drv_struct.h"
#include "mpi_mmgr.h"
#include "mpi_mem_base.h"


#define MODULE_BASE_ID (MT_ID_BUTT)

#define MEM_ADD_DEL(flag) ( (flag) > 0 ? (1) : (-1))


mt_handle g_hMemAdp = 0;

mt_s32 g_s32SMemFd = -1;

#define MEM_OPEN_FILE(fd, name)                                             \
do{                                                                         \
    if (-1 == fd)                                                           \
    {                                                                       \
        fd = open("/dev/"name, O_RDWR | O_CLOEXEC);                                     \
        if (fd < 0)                                                         \
        {                                                                   \
            MT_ERR_MEM("open %s failure, %s\n", name, strerror(errno)); \
            return MT_FAILURE;                                              \
        }                                                                   \
    }                                                                       \
}while(0)

#define MEM_CLOSE_FILE(fd)                                                  \
do{                                                                         \
    if (-1 != fd)                                                           \
    {                                                                       \
        close(fd);                                                          \
    }                                                                       \
}while(0)


#ifdef CMN_MMGR_SUPPORT
static mt_s32 mem_updata_info(mt_u32 u32ModuleID, MEM_TYPE_E enMemType, mt_s32 s32Size)
{
    mt_s32 s32Ret = MT_FAILURE;
    module_mem_info_s stModuleMem = {0};

    stModuleMem.u32ModuleID = u32ModuleID;

    switch(enMemType)
    {
        case MEM_TYPE_MMZ:
            stModuleMem.u32SizeMMZ= s32Size*MEM_ADD_DEL(s32Size);
            MT_INFO_MEM("find the module ID 0x%08x and op mmz memory size:%d\n", u32ModuleID, s32Size);
        break;
        case MEM_TYPE_USR:
            stModuleMem.u32SizeUsrMem = s32Size*MEM_ADD_DEL(s32Size);
            MT_INFO_MEM("find the module ID 0x%08x and op user memory size:%d\n", u32ModuleID, s32Size);
        break;
        case MEM_TYPE_KERNEL:
            stModuleMem.u32SizeKernelMem = s32Size*MEM_ADD_DEL(s32Size);
            MT_INFO_MEM("find the module ID 0x%08x and op kernel memory size:%d\n", u32ModuleID, s32Size);
        break;
        default:
        break;
    }

    if (s32Size < 0)
    {
        s32Ret = ioctl(g_s32SMemFd, CMD_MEM_DEL_INFO, &stModuleMem);
    }
    else
    {
        s32Ret = ioctl(g_s32SMemFd, CMD_MEM_ADD_INFO, &stModuleMem);
    }

    return s32Ret;
}
#endif

mt_s32 module_mem_adp_init(mt_u32 u32ModuleCount)
{
    MEM_OPEN_FILE(g_s32SMemFd, UMAP_DEVNAME_MEM2);

#ifdef CMN_MMGR_SUPPORT  
    mem_pool_init(MEM_POOL_COUNT, mem_updata_info);
#endif

    return MT_SUCCESS;
}

mt_s32 module_mem_adp_deinit(mt_void)
{
    MEM_CLOSE_FILE(g_s32SMemFd);

    g_s32SMemFd = -1;

#ifdef CMN_MMGR_SUPPORT
    mem_pool_deinit();
#endif
    return MT_SUCCESS;
}

mt_s32 module_mem_add_module_info(module_info_s* pstModule)
{
    mt_s32 s32Ret = MT_FAILURE;
    
    s32Ret = ioctl(g_s32SMemFd, CMD_ADD_MODULE_INFO, pstModule);

    if (MT_FAILURE == s32Ret)
    {
        return s32Ret;
    }

    return MT_SUCCESS;
}

mt_s32 module_mem_del_module_info(module_info_s* pstModule)
{
    mt_s32 s32Ret = MT_FAILURE;
    
    s32Ret = ioctl(g_s32SMemFd, CMD_DEL_MODULE_INFO, pstModule);

    if (MT_FAILURE == s32Ret)
    {
        return s32Ret;
    }

    return MT_SUCCESS;
}


