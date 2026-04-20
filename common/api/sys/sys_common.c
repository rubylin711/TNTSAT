/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <execinfo.h>

#include "mt_type.h"
#include "mt_common.h"
#include "mt_module.h"
#include "mt_osal.h"
#include "mpi_mmz.h"
#include "mt_mpi_mem.h"
#include "mt_drv_struct.h"
#include "mpi_module.h"
#include "mpi_log.h"
#include "drv_sys_ioctl.h"
#include "mt_mpi_stat.h"
#include "mpi_memdev.h"
#include "mapi_proc.h"




//////////////////////////////////////////////////////////////////////////////////////
/// STATIC CONST Variable
//////////////////////////////////////////////////////////////////////////////////////
static const mt_u8 s_szMonth[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static const mt_u8 s_szVersion[] = "SDK_VERSION:["\
                            MKMARCOTOSTR(SDK_VERSION)"] Build Time:["\
                            __DATE__", "__TIME__"]";

static pthread_mutex_t   s_SysMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t   s_tempMutex = PTHREAD_MUTEX_INITIALIZER;

/// STATIC Variable
static mt_u32 s_u32SysInitTimes = 0;
static mt_s32 s_s32SysFd = -1;

/// DEFINATION MACRO
#define SYS_OPEN_FILE                                       \
do{                                                         \
    if (-1 == s_s32SysFd)                                   \
    {                                                       \
        s_s32SysFd = open("/dev/"UMAP_DEVNAME_SYS, O_RDWR | O_CLOEXEC); \
        if (s_s32SysFd < 0)                                 \
        {                                                   \
            perror("open");                                 \
            sys_comm_unlock();                                \
            return MT_FAILURE;                                   \
        }                                                   \
    }                                                       \
}while(0)

#define sys_comm_lock()     (void)pthread_mutex_lock(&s_SysMutex);
#define sys_comm_unlock()   (void)pthread_mutex_unlock(&s_SysMutex);

#define IS_CHIP_TYPE(type) (type == SysVer.enChipTypeHardWare)
#define IS_CHIP(type, ver) ((type == SysVer.enChipTypeHardWare) && (ver == SysVer.enChipVersion))

extern int mt_mpi_sys_monitor_start(void);
extern int mt_mpi_sys_monitor_stop(void);

mt_s32 mt_sys_init(mt_void)
{
    mt_s32 s32Ret = MT_FAILURE;

    sys_comm_lock();

    if (0 == s_u32SysInitTimes ++)
    {
        SYS_OPEN_FILE;

        s32Ret = mt_mpi_log_init();
        if (MT_SUCCESS != s32Ret)
        {
            MT_FATAL_SYS("mt_mpi_log_init failure: %d\n", s32Ret);
            goto LogErrExit;
        }

        s32Ret = mt_module_init();
        if (MT_SUCCESS != s32Ret)
        {
            MT_FATAL_SYS("mt_module_init failure: %d\n", s32Ret);
            goto ModuleErrExit;
        }

        //MUST unlock, because MT_MPI_STAT_Init calling mt_sys_GetVersion will cause deadlock
        sys_comm_unlock();

        s32Ret = mt_mpi_stat_init();
        if (MT_SUCCESS != s32Ret)
        {
            MT_FATAL_SYS("mt_mpi_stat_init failure: %d\n", s32Ret);
            goto StatErrExit;
        }

        s32Ret = mpi_memdev_init();
        if (MT_SUCCESS != s32Ret)
        {
            MT_FATAL_SYS("mpi_memdev_init failure: %d\n", s32Ret);
            goto MemdevErrExit;
        }

        s32Ret = mt_api_proc_init();
        if (MT_SUCCESS != s32Ret)
        {
            MT_FATAL_SYS("proc_Init failure: %d\n", s32Ret);
            goto UprocErrExit;
        }

		mt_mpi_sys_monitor_start();

        sys_comm_lock();

        MT_INFO_SYS("mt_sys_Init init OK\n");
    }

    sys_comm_unlock();

    return (mt_s32)MT_SUCCESS;

UprocErrExit:
    (mt_void)mpi_memdev_deinit();

MemdevErrExit:
    (mt_void)mt_mpi_stat_deinit();

StatErrExit:
    /* Lock for access of s_s32SysInitTimes */
    sys_comm_lock();
    (mt_void)mt_module_deinit();

ModuleErrExit:
    mt_mpi_log_deinit();

LogErrExit:
    if (s_s32SysFd != -1)
    {
        close(s_s32SysFd);
        s_s32SysFd = -1;
    }

    s_u32SysInitTimes --;

    sys_comm_unlock();

    return (mt_s32)MT_FAILURE;
}

mt_s32 mt_sys_deinit(mt_void)
{
    sys_comm_lock();

    if (!s_u32SysInitTimes)
    {
        goto out;
    }

    if (0 == --s_u32SysInitTimes)
    {
        (mt_void)mt_api_proc_deinit();

        (mt_void)mpi_memdev_deinit();

        (mt_void)mt_mpi_stat_deinit();

        (mt_void)mt_module_deinit();

        mt_mpi_log_deinit();

		mt_mpi_sys_monitor_stop();

        if (s_s32SysFd != -1)
        {
            close(s_s32SysFd);
            s_s32SysFd = -1;
        }
    }
out:
    sys_comm_unlock();

    return MT_SUCCESS;
}

mt_s32 mt_sys_get_build_time(struct tm * pstTime)
{
    char szData[] = __DATE__;
    char szTime[] = __TIME__;
    char szTmp[5];
    int i;

    if (NULL == pstTime)
    {
        return MT_FAILURE;
    }

    /* month */
    memset(szTmp, 0, sizeof(szTmp));
    (mt_void)mt_osal_strncpy(szTmp, szData, 3);

    for(i=0; i<12; i++)
    {
            if(!mt_osal_strncmp((const char*)s_szMonth[i], szTmp, sizeof(s_szMonth[i])))
            {
                pstTime->tm_mon = i + 1;
                break;
            }
    }

    /* day */
    memset(szTmp, 0, sizeof(szTmp));
    (mt_void)mt_osal_strncpy(szTmp, szData+4, 2);
    pstTime->tm_mday = atoi(szTmp);

    /* year */
    memset(szTmp, 0, sizeof(szTmp));
    (mt_void)mt_osal_strncpy(szTmp, szData+7, 4);
    pstTime->tm_year = atoi(szTmp);

    /* hour */
    memset(szTmp, 0, sizeof(szTmp));
    (mt_void)mt_osal_strncpy(szTmp, szTime, 2);
    pstTime->tm_hour = atoi(szTmp);

    /* minute */
    memset(szTmp, 0, sizeof(szTmp));
    (mt_void)mt_osal_strncpy(szTmp, szTime+3, 2);
    pstTime->tm_min = atoi(szTmp);


    /* second */
    memset(szTmp, 0, sizeof(szTmp));
    (mt_void)mt_osal_strncpy(szTmp, szTime+6, 2);
    pstTime->tm_sec = atoi(szTmp);

    return MT_SUCCESS;
}

mt_s32 mt_sys_get_version(mt_sys_version_s *pstVersion)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (NULL == pstVersion)
    {
        return MT_FAILURE;
    }

#ifdef CONFIG_MT_CHIP_ARIA
    pstVersion->enChipTypeHardWare = MT_CHIP_TYPE_MT_ARIA;
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    pstVersion->enChipTypeHardWare = MT_CHIP_TYPE_MT_SYMPHONY;
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
    pstVersion->enChipTypeHardWare = MT_CHIP_TYPE_MT_SYMPHONY4;
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
    pstVersion->enChipTypeHardWare = MT_CHIP_TYPE_MT_SYMPHONY6;
#else
    pstVersion->enChipTypeHardWare = MT_CHIP_TYPE_BUTT;
#endif

    sys_comm_lock();

    if (s_s32SysFd < 0)
    {
        sys_comm_unlock();
        return MT_FAILURE;
    }

    s32Ret = ioctl(s_s32SysFd, SYS_GET_SYS_VERSION, pstVersion);
    if (s32Ret != 0)
    {
        sys_comm_unlock();

        MT_ERR_SYS("ioctl SYS_GET_SYS_VERSION error!\n");
        return MT_FAILURE;
    }

    (mt_void)mt_osal_snprintf(pstVersion->aversion, sizeof(pstVersion->aversion), "%s", s_szVersion);

    /* for detect soft and chip dismatch */

    pstVersion->enChipTypeSoft = MT_CHIP_TYPE_MT_ARIA;

    sys_comm_unlock();

    return MT_SUCCESS;
}

static mt_s32 get_chip_cap_support_helper(MT_CHIP_CAP_E enChipCap, mt_u32 *pu32Support)
{
    mt_s32 s32Ret;

    if (!pu32Support)
        return MT_FAILURE;

    sys_comm_lock();

    if (s_s32SysFd < 0)
    {
        s32Ret = MT_FAILURE;
        goto out;
    }

    switch(enChipCap)
    {
        case MT_CHIP_CAP_DOLBY:
            s32Ret = ioctl(s_s32SysFd, SYS_GET_DOLBYSUPPORT, pu32Support);
            break;
        case MT_CHIP_CAP_DTS:
            s32Ret =  ioctl(s_s32SysFd, SYS_GET_DTSSUPPORT, pu32Support);
            break;
        case MT_CHIP_CAP_ADVCA:
            s32Ret =  ioctl(s_s32SysFd, SYS_GET_ADVCASUPPORT, pu32Support);
            break;
        case MT_CHIP_CAP_MACROVISION:
            s32Ret =  ioctl(s_s32SysFd, SYS_GET_MACROVISIONSUPPORT, pu32Support);
            break;
        default:
            MT_ERR_SYS("invalid chip attrs type!\n");
            s32Ret = MT_FAILURE;
            break;
    }

out:
    sys_comm_unlock();

    return s32Ret;
}

mt_s32 mt_sys_get_chip_capability(MT_CHIP_CAP_E enChipCap, MT_BOOL *pbSupport)
{
    mt_s32  s32Ret;
    mt_u32 u32Support = 0;

    if (!pbSupport)
    {
        MT_ERR_SYS("null ptr!\n");
        return MT_FAILURE;
    }

    s32Ret = get_chip_cap_support_helper(enChipCap, &u32Support);

    if (s32Ret == MT_SUCCESS)
    {
        *pbSupport = (MT_BOOL)u32Support;
    }

    return s32Ret;
}

mt_s32 mt_sys_get_chip_attr(mt_sys_chip_attr_s * pst_chipattr)
{
    mt_s32              s32Ret = MT_FAILURE;
    mt_u32              u32Support = 0;

    if (!pst_chipattr)
    {
        MT_ERR_SYS("null ptr!\n");
        return MT_FAILURE;
    }

    s32Ret = get_chip_cap_support_helper(MT_CHIP_CAP_DOLBY, &u32Support);
    pst_chipattr->bDolbySupport = (MT_BOOL)u32Support;
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_SYS("Get ChipAttr DOLBY error!\n");
        return MT_FAILURE;
    }
#if 0
    s32Ret = get_chip_cap_support_helper(MT_CHIP_CAP_DTS, &u32Support);
    pst_chipattr->bDTSSupport = (MT_BOOL)u32Support;
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_SYS("Get ChipAttr DTS error!\n");
        return MT_FAILURE;
    }

    s32Ret = get_chip_cap_support_helper(MT_CHIP_CAP_ADVCA, &u32Support);
    pst_chipattr->bADVCASupport = (MT_BOOL)u32Support;
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_SYS("Get ChipAttr ADVCA error!\n");
        return MT_FAILURE;
    }

    s32Ret = get_chip_cap_support_helper(MT_CHIP_CAP_MACROVISION, &u32Support);
    pst_chipattr->bMacrovisionSupport = (MT_BOOL)u32Support;
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_SYS("Get ChipAttr MACROVISION error!\n");
        return MT_FAILURE;
    }

    sys_comm_lock();
    s32Ret = ioctl(s_s32SysFd, SYS_GET_DIEID, &(pst_chipattr->u64ChipID));
    if (MT_SUCCESS != s32Ret)
    {
        sys_comm_unlock();
        MT_ERR_SYS("Get ChipAttr DIEID error!\n");
        return MT_FAILURE;
    }
    sys_comm_unlock();
#endif
    return MT_SUCCESS;

}

mt_s32 mt_sys_crc32(mt_u8 *pu8Src, mt_u32 u32SrcLen, ulong *pDst)
{
    mt_s32          Ret = MT_SUCCESS;
    unsigned long   poly_nomial = 0x04c11db7;
    short           First = 1;              /*Init _CrcTable[256] Flag, Only init once time*/
    unsigned long   CrcTable[256];          /*Calculate CRC32*/
    unsigned long   crc = 0xffffffff;

    if(First)
    {
        int    i, j;
        unsigned long crc_accum;

        for (i = 0;  i < 256;  i++)
        {
            crc_accum =  ( (unsigned long)i << 24 );

            for (j = 0; j < 8; j++)
            {
                if( crc_accum & 0x80000000L )
                    crc_accum = ( crc_accum << 1 ) ^ poly_nomial;
                else
                    crc_accum = ( crc_accum << 1 );
            }

            CrcTable[i] = crc_accum;
        }

        First = 0;
    }

    while (u32SrcLen--)
    {
        crc = (crc << 8) ^ CrcTable[((crc >> 24) ^ *pu8Src++) & 0xff];
    }

    *pDst = crc;

    return Ret;
}

mt_s32 mt_sys_get_mem_config(mt_sys_mem_config_s *pstConfig)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (NULL == pstConfig)
    {
        MT_ERR_SYS("Get DDRConf ptr!\n");

        return MT_FAILURE;
    }

    sys_comm_lock();

    if (s_s32SysFd < 0)
    {
        sys_comm_unlock();
        return MT_FAILURE;
    }

    s32Ret = ioctl(s_s32SysFd, SYS_GET_DDRCONFIG, pstConfig);

    sys_comm_unlock();

    return s32Ret;
}

mt_s32 mt_sys_set_conf(const mt_sys_conf_s *pstSysConf)
{
    mt_s32 s32Ret = MT_FAILURE;

    if ( NULL == pstSysConf )
    {
        MT_ERR_SYS("Set pstSysConf ptr!\n");

        return MT_FAILURE;
    }

    sys_comm_lock();

    if (s_s32SysFd < 0)
    {
        sys_comm_unlock();
        return MT_FAILURE;
    }

    s32Ret = ioctl(s_s32SysFd, SYS_SET_CONFIG_CTRL, pstSysConf);

    sys_comm_unlock();

    return s32Ret;
}

mt_s32 mt_sys_get_conf(mt_sys_conf_s *pstSysConf)
{
    mt_s32 s32Ret = MT_FAILURE;

    if ( NULL == pstSysConf )
    {
        MT_ERR_SYS("Get pstSysConf ptr!\n");

        return MT_FAILURE;
    }

    sys_comm_lock();

    if (s_s32SysFd < 0)
    {
        sys_comm_unlock();
        return MT_FAILURE;
    }

    s32Ret = ioctl(s_s32SysFd, SYS_GET_CONFIG_CTRL, pstSysConf);

    sys_comm_unlock();

    return s32Ret;
}

mt_s32 mt_sys_write_register(phys_addr_t RegAddr, mt_u32 u32Value)
{
    return mpi_memdev_write_register(RegAddr, u32Value);
}

mt_s32 mt_sys_read_register(phys_addr_t RegAddr, mt_u32 *pu32Value)
{
    return mpi_memdev_read_register(RegAddr, pu32Value);
}

mt_s32 mt_sys_map_register(phys_addr_t RegAddr, mt_u32 u32Length, mt_void **pVirAddr)
{
    return mpi_memdev_map_register(RegAddr, u32Length, pVirAddr);
}

mt_s32 mt_sys_unmap_register(mt_void * pVirAddr)
{
    return mpi_memdev_unmap_register(pVirAddr);
}

mt_s32 mt_sys_get_time_stamp_ms(mt_u32 *pu32TimeMs)
{
    mt_s32 s32Ret = MT_FAILURE;

    if (MT_NULL == pu32TimeMs)
    {
        MT_ERR_SYS("null pointer error\n");
        return MT_FAILURE;
    }

    sys_comm_lock();

    if (s_s32SysFd < 0)
    {
        sys_comm_unlock();
        return MT_FAILURE;
    }

    s32Ret = ioctl(s_s32SysFd, SYS_GET_TIMESTAMPMS, pu32TimeMs);

    sys_comm_unlock();

    return s32Ret;
}

mt_s32 mt_sys_set_log_level(mt_mod_id_e enModId,  mt_log_level_e enLogLevel)
{
    return mt_mpi_log_level_set((mt_u32)enModId, enLogLevel);
}

mt_s32 mt_sys_set_log_path(const mt_char* pszLogPath)
{
    return mt_mpi_log_path_set(pszLogPath);
}

mt_s32 mt_sys_set_store_path(const mt_char* pszPath)
{
    return mt_mpi_log_storepath_set(pszPath);
}

mt_s32 mt_proc_add_dir(const mt_char * pszName)
{
    return  mt_api_proc_add_dir(pszName);
}

mt_s32 mt_proc_remove_dir(const mt_char *pszName)
{
    return mt_api_proc_rm_dir(pszName);
}

mt_s32 mt_proc_add_entry(mt_u32 u32ModuleID, const mt_u_proc_entry_s* pstEntry)
{
    return mt_api_proc_add_entry(u32ModuleID, pstEntry);
}

mt_s32 mt_proc_remove_entry(mt_u32 u32ModuleID, const mt_u_proc_entry_s* pstEntry)
{
    return mt_api_proc_rm_entry(u32ModuleID, pstEntry);
}

mt_s32 mt_proc_printf(mt_proc_show_buffer_s *pstBuf, const mt_char *pFmt, ...)
{
    mt_u32 u32Len = 0;
    va_list args = {0};

    if ((MT_NULL == pstBuf) || (MT_NULL == pstBuf->pu8Buf) || (MT_NULL == pFmt))
    {
        return MT_FAILURE;
    }

    /* log buffer overflow */
    if (pstBuf->offset >= pstBuf->size)
    {
        MT_ERR_SYS("userproc log buffer(size:%d) overflow.\n", pstBuf->size);
        return MT_FAILURE;
    }

    va_start(args, pFmt);
    u32Len = (mt_u32)mt_osal_vsnprintf((mt_char*)pstBuf->pu8Buf + pstBuf->offset,
                            pstBuf->size - pstBuf->offset, pFmt, args);
    va_end(args);

    pstBuf->offset += u32Len;

    return MT_SUCCESS;
}

mt_s32 mt_mmz_malloc(mt_mmz_buf_s *pstBuf)
{
    return mt_mpi_mmz_malloc(pstBuf);
}

mt_s32 mt_mmz_free(mt_mmz_buf_s *pstBuf)
{
    return mt_mpi_mmz_free(pstBuf);
}

phys_addr_t mt_mmz_new(ulong size , mt_u32 u32Align, const mt_char *ps8MMZName, const mt_char *ps8MMBName)
{
    return mt_mpi_mmz_new(size, u32Align, ps8MMZName, ps8MMBName, NULL);
}

phys_addr_t mt_mmz_new_secure(ulong size , mt_u32 u32Align, const mt_char *ps8MMZName, const mt_char *ps8MMBName,
							mt_mmz_security_attr_s *attr)
{
    return mt_mpi_mmz_new(size, u32Align, ps8MMZName, ps8MMBName, attr);
}

mt_s32 mt_mmz_delete(phys_addr_t PhysAddr)
{
    return mt_mpi_mmz_delete(PhysAddr);
}

mt_void *mt_mmz_map(phys_addr_t PhysAddr, mt_u32 u32Cached)
{
    return mt_mpi_mmz_map(PhysAddr, u32Cached);
}

mt_s32 mt_mmz_unmap(void *vAddr)
{
    return mt_mpi_mmz_unmap(vAddr);
}

mt_s32 mt_mmz_flush(void *vAddr, ulong offset, ulong size)
{
    return mt_mpi_mmz_flush(vAddr, offset, size);
}

mt_s32 mt_mmz_invalidate(void *vAddr, ulong offset, ulong size)
{
    return mt_mpi_mmz_invalidate(vAddr, offset, size);
}

mt_s32 mt_mmz_get_phyaddr(void *vaddr, phys_addr_t *Phyaddr, ulong *Size)
{
    return mt_mpi_mmz_getphyaddr(vaddr, Phyaddr, Size);
}

mt_s32 mt_mmz_get_start_size(const char *mmz_name, phys_addr_t *phys_start, ulong *size)
{
	return mt_mpi_mmz_get_start_size(mmz_name, phys_start, size);
}

mt_void *mt_mem_map(phys_addr_t PhyAddr, ulong size)
{
    return mt_mmap(PhyAddr, size);
}

mt_void *mt_mem_map_cache(phys_addr_t PhyAddr, ulong size)
{
    return mt_mmap_cache(PhyAddr, size);
}

mt_s32 mt_mem_unmap(void *AddrMapped)
{
    return mt_munmap(AddrMapped);
}

mt_s32 mt_mem_flush(void *virtaddr, ulong size)
{
	return mt_flush(virtaddr, size);
}

mt_s32 mt_mem_invalidate(void *virtaddr, ulong size)
{
	return mt_invalidate(virtaddr, size);
}

mt_s32 mt_mem_get_pageinfo(mt_u32 *page_block_order, mt_u32 *pages_per_block)
{
	return mt_get_pageinfo(page_block_order, pages_per_block);
}

mt_s32 mt_mem_get_phyaddr(void *vaddr, phys_addr_t *phy_addr)
{
    return mt_get_phys_addr(vaddr, phy_addr);
}

mt_void* mt_mem_malloc(mt_u32 u32ModuleID, mt_u32 u32Size)
{
    return mt_malloc(u32ModuleID, u32Size);
}

mt_void mt_mem_free(mt_u32 u32ModuleID, mt_void* pMemAddr)
{
    mt_free(u32ModuleID, pMemAddr);
}

mt_void* mt_mem_calloc(mt_u32 u32ModuleID, mt_u32 u32MemBlock, mt_u32 u32Size)
{
    return mt_calloc(u32ModuleID, u32MemBlock, u32Size);
}

mt_void* mt_mem_realloc(mt_u32 u32ModuleID, mt_void *pMemAddr, mt_u32 u32Size)
{
    return mt_realloc(u32ModuleID, pMemAddr, u32Size);
}

#if (defined(__GNUC__) && (__GNUC__ < 10))
pid_t gettid(void)
{
    return syscall(__NR_gettid);
}
#endif

long sched_setattr(pid_t pid, struct sched_attr *attr,
				  unsigned int flags)
{
    return syscall(__NR_sched_setattr, pid, attr, flags);
}

long sched_getattr(pid_t pid, struct sched_attr *attr,
				  unsigned int size, unsigned int flags)
{
    return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

#define TASK_COMM_LEN 16
int mt_set_pthread_name(const char *name)
{
	int ret = 0;
	char tname[TASK_COMM_LEN]={0};
	if(!name)
	{
		return -1;
	}
	snprintf(tname, TASK_COMM_LEN, "%s", name);
	ret = prctl(PR_SET_NAME, tname);
	return ret;
}

int daemon_init(void)
{
	int pid;
	int i;
	long openmax;

	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGCONT, SIG_IGN);
	signal(SIGSTOP, SIG_IGN);

	pid = fork();
	if (pid > 0) {
		exit(0);
	} else if (pid < 0) {
		return -1;
	}

	setsid();

	pid = fork();
	if (pid > 0) {
		exit(0);
	} else if (pid < 0) {
		return -1;
	}

	openmax = sysconf(_SC_OPEN_MAX);
	for (i = 0; i < openmax; i++) {
		close(i);
	}
	open("/dev/null", O_RDWR | O_CLOEXEC);
	dup(0);
	dup(0);

	chdir("/");

	umask(0);

	signal(SIGCHLD, SIG_IGN);

	return 0;
}

int mt_msleep(mt_u32 msecs)
{
	struct timespec ts;
	int ret;

	if (msecs == 0) {
		return 0;
	}

	ts.tv_sec = (long)(msecs / 1000);
	ts.tv_nsec = (long)(msecs % 1000) * 1000000;
	while (((ret = nanosleep(&ts, &ts)) == -1) && errno == EINTR);
	return ret;
}

/* http://man7.org/linux/man-pages/man2/clock_nanosleep.2.html */
static int mt_clock_nanosleep_debug(clockid_t clock_id, int flags,
                                    const struct timespec *request,
                                    struct timespec *remain,
                                    const char *func, int line)
{
#ifdef CONFIG_MT_DEBUG_SLEEP
	static unsigned long long sleep_times     = 0;
	static unsigned long long failed_times    = 0;
	static unsigned long long interrupt_times = 0;
#endif

	int ret;

#ifdef CONFIG_MT_DEBUG_SLEEP
	sleep_times ++;
#endif

#ifdef CONFIG_MT_USE_CLOCK_NANOSLEEP
	ret = clock_nanosleep(clock_id, flags, request, remain);
#else
	ret = nanosleep(request, remain);
#endif

#ifdef CONFIG_MT_DEBUG_SLEEP
	if (ret < 0)
	{
		failed_times ++;

		if (errno == EINTR)
		{
			interrupt_times ++;

			if ((interrupt_times % 100) == 0)
			{
				MT_ERR_SYS("%s@%d: total %llu, failed %llu, interrupted %llu\n",
					func,line,
					sleep_times,failed_times,interrupt_times);
			}
		}
		else
		{
			MT_ERR_SYS("%s@%d: nanosleep failed, retval: %d\n",func,line,ret);
		}
	}
#endif

	return ret;
}

/* http://man7.org/linux/man-pages/man2/nanosleep.2.html */
int mt_nanosleep_debug(const struct timespec *req, struct timespec *rem, const char *func, int line)
{
	//FIXME: CLOCK_PROCESS_CPUTIME_ID not work if process sleeped!
	//return mt_clock_nanosleep(CLOCK_PROCESS_CPUTIME_ID, 0, req, rem);
	return mt_clock_nanosleep_debug(CLOCK_MONOTONIC, 0, req, rem, func, line);
}

/* http://man7.org/linux/man-pages/man3/usleep.3.html */
int mt_usleep_debug(unsigned int usec, const char *func, int line)
{
//#ifdef CONFIG_MT_DEBUG_SLEEP

	#define SLEEP_THRESHOLD_US	1000

	if (usec < SLEEP_THRESHOLD_US)
	{
		MT_ERR_SYS("[W]>>>%s@%d: usec(%u) < %d! if you need sleep < %dus, pls call MT_NANOSLEEP!\n",
			func,line,usec,SLEEP_THRESHOLD_US,SLEEP_THRESHOLD_US);

		//debug
		if (1)
		{
			abort();
		}
	}
//#endif

#if defined(CONFIG_MT_USE_CLOCK_NANOSLEEP) || defined(CONFIG_MT_USE_NANOSLEEP)

	struct timespec ts = {0};
	int ret;

	ts.tv_sec = (long)usec / 1000000;
	ts.tv_nsec = ((long)usec % 1000000) * 1000;

	while (((ret=mt_nanosleep_debug(&ts, &ts, func, line)) < 0)
			&& (errno == EINTR))
	{
		continue;
	}

	return ret;

#elif defined(CONFIG_MT_USE_USLEEP)

#ifdef CONFIG_MT_DEBUG_SLEEP
	MT_ERR_SYS("[W]>>>%s@%d: usec(%u). usleep is obsolete!!!\n",
		func,line,usec);
#endif

	return usleep(usec);

#else  /* CONFIG_MT_USE_MSLEEP */

	unsigned int msecs;

	msecs = (usec + 500) / 1000;

	//patch
	if (msecs == 0)
	{
		msecs = 1;
	}

	return mt_msleep((mt_u32)msecs);

#endif
}

#define BACKTRACE_SIZ 100
void mt_backtrace(void)
{
	void *array[BACKTRACE_SIZ];
	int size, i;
	char **strings;

	size = backtrace(array, BACKTRACE_SIZ);
	strings = backtrace_symbols(array, size);

	MT_PRINT("size = %ld\n", (long)size);
	for (i = 0; i < size; ++i) {
		MT_PRINT("%p : %s\n", array[i], strings[i]);
	}

	MT_PRINT("---------------------------------------------------------\n");
	free(strings);
}


mt_s32 mt_sys_set_avdate(mt_sysdate_t *date)
{
	//MT_ERR_SYS("mt_sys_set_avdate\n");

	mt_s32 s32Ret = MT_FAILURE;

    if (NULL == date)
    {
        MT_ERR_SYS("Set Avdate ptr!\n");

        return MT_FAILURE;
    }
    // MT_ERR_SYS("\ny:%d,m:%d,d:%d,h:%d,m:%d,s:%d\n",date->tm_year,date->tm_mon,date->tm_mday,date->tm_hour,date->tm_min,date->tm_sec);


    sys_comm_lock();

    if (s_s32SysFd < 0)
    {
        sys_comm_unlock();
        return MT_FAILURE;
    }

    s32Ret = ioctl(s_s32SysFd, SYS_SET_AVDATE, date);
   // MT_ERR_SYS("Set s32Ret:%d\n",s32Ret);
    sys_comm_unlock();

    return s32Ret;
}

mt_s32 mt_sys_get_avdate(mt_sysdate_t *date)
{
	// MT_ERR_SYS("mt_sys_get_avdate\n");
	mt_s32 s32Ret = MT_FAILURE;

    if (NULL == date)
    {
        MT_ERR_SYS("Get Avdate ptr!\n");

        return MT_FAILURE;
    }

    sys_comm_lock();

    if (s_s32SysFd < 0)
    {
        sys_comm_unlock();
        return MT_FAILURE;
    }

    s32Ret = ioctl(s_s32SysFd, SYS_GET_AVDATE, date);
	// MT_ERR_SYS("\ny:%d,m:%d,d:%d,h:%d,m:%d,s:%d\n",date->tm_year,date->tm_mon,date->tm_mday,date->tm_hour,date->tm_min,date->tm_sec);

    sys_comm_unlock();

    return s32Ret;
}

/*
 * integer: temp / 1000
 * decimal: temp % 1000
 */
mt_s32 mt_sys_get_temp(int *temp)
{
	mt_s32 s32Ret = MT_FAILURE;

    if (NULL == temp)
    {
        MT_ERR_SYS("temp is null!\n");

        return MT_FAILURE;
    }

	(void)pthread_mutex_lock(&s_tempMutex);

    if (s_s32SysFd < 0)
    {
        sys_comm_unlock();
        return MT_FAILURE;
    }

    s32Ret = ioctl(s_s32SysFd, SYS_GET_TEMP, temp);

    (void)pthread_mutex_unlock(&s_tempMutex);

    return s32Ret;
}

static struct timespec ticksStartTime = {0, 0};

MT_BOOL mt_ticks_init(u32 cpu_freq)
{
    clock_gettime(CLOCK_MONOTONIC, &ticksStartTime);
    return TRUE;
}

u32 mt_ticks_get(void)
{
    U64 ms = 0;
#if 1 /***relative time mode****/
    struct timespec ticksTime = {0, 0};
    struct timespec distTime = {0, 0};
    U64 tmpnsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &ticksTime);

    if(ticksStartTime.tv_nsec > ticksTime.tv_nsec)
      {
         ticksTime.tv_sec -= 1;
         tmpnsec = 1000 * 1000 * 1000;
         tmpnsec += (U64)ticksTime.tv_nsec;
      }
    else
      {
        tmpnsec = (U64)ticksTime.tv_nsec;
      }

    distTime.tv_nsec = (long)tmpnsec - ticksStartTime.tv_nsec;

    if(ticksTime.tv_sec > ticksStartTime.tv_sec)
      {
         distTime.tv_sec =  ticksTime.tv_sec - ticksStartTime.tv_sec;
         ms = (U64)distTime.tv_sec * 1000;
         ms += (U64)distTime.tv_nsec / 1000000;

      }
    else
      {
         ms = 0;
         printf("@@@@@@@@@@@@mt_ticks_get is err,it don't should  happen!!!!!!!!!");
      }
#else
    ms = SYS_GetMS();
#endif


    ms = ms / 10;
    return (U32)ms;
}

