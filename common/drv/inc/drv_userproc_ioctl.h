/******************************************************************************

 
******************************************************************************/

#ifndef __DRV_USERPROC_IOCTL_H__
#define __DRV_USERPROC_IOCTL_H__

/******************************* Include Files *******************************/

/* add include here */
#include "mt_type.h"
#include "mt_drv_mmz.h"
#include "mt_drv_userproc.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/

#define MT_UPROC_READ_CMD "__read"

/*************************** Structure Definition ****************************/

typedef struct 
{
    mt_char aszName[MAX_PROC_NAME_LEN + 1];     /* Input, entry name */
    mt_char aszParent[MAX_PROC_NAME_LEN + 1];   /* Input, directory name */
    mt_proc_show_fn pfnShowFunc;                /* Input, show function */
    mt_proc_cmd_fn pfnCmdFunc;                  /* Input, cmd function */
    mt_void * pPrivData;                         /* Input, private data*/
    mt_void *pEntry;                            /* Output, entry pointer */
    mt_void *pFile;                             /* Output, Belongs to which file */
    mmz_buffer_s stBuf;                         /* Output, buffer */
}mt_drv_u_proc_entry_t;

typedef struct USRMODEPROC_CMD_S
{
    mt_void     *pEntry;                        /* The type is mt_drv_u_proc_entry_t* */
    mt_s32      s32Write;
    mt_char     aszCmd[MAX_PROC_CMD_LEN + 1];
}mt_drv_u_proc_cmd_t;

typedef struct USRMODEPROC_CMDINFO_S
{
    mt_drv_u_proc_cmd_t stCmd;
    mt_drv_u_proc_entry_t stEntry;
}mt_drv_u_proc_cmdinfo_t;

typedef char mt_proc_dir_name_t[MAX_PROC_NAME_LEN+1];

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/

#define	UMPIOC_ADD_ENTRY		           _IOWR(MT_ID_PROC, 1, mt_drv_u_proc_entry_t)
#define	UMPIOC_REMOVE_ENTRY		    _IOW(MT_ID_PROC, 2, mt_drv_u_proc_entry_t)
#define	UMPIOC_ADD_DIR		           _IOW(MT_ID_PROC, 3, mt_proc_dir_name_t)
#define	UMPIOC_REMOVE_DIR                 _IOW(MT_ID_PROC, 4, mt_proc_dir_name_t)
#define	UMPIOC_GETCMD   		           _IOR(MT_ID_PROC, 5, mt_drv_u_proc_cmdinfo_t)
#define	UMPIOC_WAKE_READ_TASK   	     _IOW(MT_ID_PROC, 6, mt_s32)
#define	UMPIOC_WAKE_WRITE_TASK        _IOW(MT_ID_PROC, 7, mt_s32)

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __DRV_USERPROC_IOCTL_H__ */


