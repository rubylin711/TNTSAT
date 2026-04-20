/******************************************************************************


******************************************************************************/

#ifndef __MT_DRV_UPROC_H__
#define __MT_DRV_UPROC_H__

/******************************* Include Files *******************************/

/* add include here */
#include "mt_type.h"
#include "mt_module.h"
#include "mt_debug.h"
#include "mt_drv_struct.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/

#define MAX_PROC_NAME_LEN       127
#define MAX_PROC_CMD_LEN        255
#define MT_PROC_BUFFER_SIZE (4096)

#define MT_USERPROC_DEVNAME     UMAP_NAME"userproc"

#define  PROC_FATAL(fmt...)   MT_FATAL_PRINT(MT_ID_PROC, fmt)
#define  PROC_ERR(fmt...)     MT_ERR_PRINT(MT_ID_PROC, fmt)
#define  PROC_WARN(fmt...)    MT_WARN_PRINT(MT_ID_PROC, fmt)
#define  PROC_INFO(fmt...)    MT_INFO_PRINT(MT_ID_PROC, fmt)

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/
#if !(0 == MT_PROC_SUPPORT)
mt_s32 mt_proc_drv_mod_init(mt_void);
mt_void mt_proc_drv_mod_exit(mt_void);
#else
static inline mt_s32 mt_proc_drv_mod_init(mt_void)
{
    return MT_SUCCESS;
}
static inline mt_void mt_proc_drv_mod_exit(mt_void)
{
    return;
}
#endif


#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MT_DRV_USERPROC_H__ */
