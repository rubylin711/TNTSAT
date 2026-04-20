/******************************************************************************




******************************************************************************/

#ifndef __MPI_USERPROC_H__
#define __MPI_USERPROC_H__

/******************************* Include Files *******************************/

/* add include here */
#include "mt_type.h"
#include "mt_module.h"
#include "mt_debug.h"
#include "mt_common.h"

#include "mt_drv_userproc.h"
#include "drv_userproc_ioctl.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/
#if !(0 == MT_PROC_SUPPORT)
mt_s32 mt_api_proc_init(mt_void);
mt_s32 mt_api_proc_deinit(mt_void);
mt_s32 mt_api_proc_add_dir(const mt_char *pszName);
mt_s32 mt_api_proc_rm_dir(const mt_char *pszName);
mt_s32 mt_api_proc_add_entry(mt_u32 u_id, const mt_u_proc_entry_s* pst_entry);
mt_s32 mt_api_proc_rm_entry(mt_u32 u_id, const mt_u_proc_entry_s* pst_entry);
#else
static inline mt_s32 mt_api_proc_init(mt_void)
{
    return MT_SUCCESS;
}
static inline mt_s32 mt_api_proc_deinit(mt_void)
{
    return MT_SUCCESS;
}
static inline mt_s32 mt_api_proc_add_dir(const mt_char *pszName)
{
    return MT_SUCCESS;
}
static inline mt_s32 mt_api_proc_rm_dir(const mt_char *pszName)
{
    return MT_SUCCESS;
}
static inline mt_s32 mt_api_proc_add_entry(mt_u32 u_id, const mt_u_proc_entry_s* pst_entry)
{
    return MT_SUCCESS;
}
static inline mt_s32 mt_api_proc_rm_entry(mt_u32 u_id, const mt_u_proc_entry_s* pst_entry)
{
    return MT_SUCCESS;
}
#endif

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MPI_USERPROC_H__ */

