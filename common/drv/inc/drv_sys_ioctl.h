#ifndef  __DRV_SYS_IOCTL_H__
#define  __DRV_SYS_IOCTL_H__

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

#include "mt_debug.h"
#include "mt_drv_sys.h"

typedef enum hiIOC_NR_SYS_E
{
    IOC_NR_SYS_INIT = 0,
    IOC_NR_SYS_EXIT,
    IOC_NR_SYS_SETCONFIG,
    IOC_NR_SYS_GETCONFIG,
    IOC_NR_SYS_GETVERSION,
    IOC_NR_SYS_GETTIMESTAMPMS,
    IOC_NR_SYS_GETDOLBYSUPPORT,
    IOC_NR_SYS_GETDTSSUPPORT,
    IOC_NR_SYS_GETADVCASUPPORT,
    IOC_NR_SYS_GETMACROVISIONSUPPORT,
    IOC_NR_SYS_GETDDRCONFIG,
    IOC_NR_SYS_GETDIEID,
    IOC_NR_SYS_SETAVDATE,
    IOC_NR_SYS_GETAVDATE,
	IOC_NR_SYS_GETTEMP,					/* get temperature */
} IOC_NR_SYS_E;


#define SYS_INIT_CTRL         			_IO (MT_ID_SYS, IOC_NR_SYS_INIT)
#define SYS_EXIT_CTRL         			_IO (MT_ID_SYS, IOC_NR_SYS_EXIT)
#define SYS_SET_CONFIG_CTRL   			_IOW(MT_ID_SYS, IOC_NR_SYS_SETCONFIG, mt_sys_conf_s)
#define SYS_GET_CONFIG_CTRL   			_IOR(MT_ID_SYS, IOC_NR_SYS_GETCONFIG, mt_sys_conf_s)
#define SYS_GET_SYS_VERSION   			_IOWR(MT_ID_SYS, IOC_NR_SYS_GETVERSION, mt_sys_version_s)
#define SYS_GET_TIMESTAMPMS   			_IOR(MT_ID_SYS, IOC_NR_SYS_GETTIMESTAMPMS, mt_u32)
#define SYS_GET_DOLBYSUPPORT   			_IOR(MT_ID_SYS, IOC_NR_SYS_GETDOLBYSUPPORT, mt_u32)
#define SYS_GET_DTSSUPPORT   			_IOR(MT_ID_SYS, IOC_NR_SYS_GETDTSSUPPORT, mt_u32)
#define SYS_GET_ADVCASUPPORT   			_IOR(MT_ID_SYS, IOC_NR_SYS_GETADVCASUPPORT, mt_u32)
#define SYS_GET_MACROVISIONSUPPORT  	_IOR(MT_ID_SYS, IOC_NR_SYS_GETMACROVISIONSUPPORT, mt_u32)
#define SYS_GET_DDRCONFIG         		_IOR(MT_ID_SYS, IOC_NR_SYS_GETDDRCONFIG, mt_sys_mem_config_s)
#define SYS_GET_DIEID               	_IOR(MT_ID_SYS, IOC_NR_SYS_GETDIEID, mt_u64)
#define SYS_SET_AVDATE         			_IOW(MT_ID_SYS, IOC_NR_SYS_SETAVDATE, mt_sysdate_t)
#define SYS_GET_AVDATE              	_IOR(MT_ID_SYS, IOC_NR_SYS_GETAVDATE, mt_sysdate_t)
#define SYS_GET_TEMP                	_IOR(MT_ID_SYS, IOC_NR_SYS_GETTEMP, int)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRV_SYS_IOCTL_H__ */

