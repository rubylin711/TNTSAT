#ifndef __DRV_STAT_IOCTL_H__
#define __DRV_STAT_IOCTL_H__

#include "mt_debug.h"
#include "mt_drv_stat.h"

typedef struct
{
    STAT_EVENT_E  enEvent;
    
    mt_u32        Value;
    
}stat_event_s;


#define UMAPC_CMPI_STAT_REGISTER           _IOWR (MT_ID_STAT, 101, mt_u32 *)
#define UMAPC_CMPI_STAT_UNREGISTER         _IOW (MT_ID_STAT,  102, mt_u32)

#define UMAPC_CMPI_STAT_RESETALL           _IO (MT_ID_STAT,   103)

#define UMAPC_CMPI_STAT_EVENT              _IOW (MT_ID_STAT,  104, stat_event_s)
#define UMAPC_CMPI_STAT_GETTICK            _IOR (MT_ID_STAT,  105, mt_u32)

#define UMAPC_CMPI_STAT_LD_EVENT        _IOW(MT_ID_STAT, 106, mt_ld_event_s)

#endif /* __DRV_STAT_IOCTL_H__ */

