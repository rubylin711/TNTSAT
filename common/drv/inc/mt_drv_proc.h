/******************************************************************************

******************************************************************************/

#ifndef __MT_DRV_PROC_H__
#define __MT_DRV_PROC_H__

#include <linux/seq_file.h>
#include "mt_type.h"

#ifdef __cplusplus
extern "C"
{
#endif 

/** @addtogroup H_PROC */
/** @{ */

//#
#ifdef MT_PROC_SUPPORT

#define PROC_PRINT(s,arg...) ({seq_printf(s,arg) ;\
                               0 ; })


mt_void mt_drv_proc_echohelp(const mt_char *fmt, ...);
mt_void mt_drv_proc_echohelpvargs(mt_char *buf, mt_u32 size,  const mt_char * fmt, va_list args);

#else

#define PROC_PRINT(arg...) ({do{}while(0);0;}) 

static inline mt_void mt_drv_proc_echohelp(const mt_char *fmt, ...) { }
static inline mt_void mt_drv_proc_echohelpvargs(mt_char *buf, mt_u32 size,  const mt_char * fmt, va_list args) { }

#endif

#define MAX_ENTRY_NAME_LEN (31)

typedef mt_s32 (*mt_proc_ctrl_func)(mt_u32, mt_u32);

typedef mt_s32 (*mt_proc_read_func)(struct seq_file *, mt_void *);
typedef mt_s32 (*mt_drv_proc_write_func)(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos);
typedef mt_s32 (*mt_drv_proc_ioctl_func)(struct seq_file *, mt_u32 cmd, mt_u32 arg);

typedef struct tag_mt_proc_entry
{
    mt_char entry_name[MAX_ENTRY_NAME_LEN+1];
    struct proc_dir_entry *entry;
    mt_proc_read_func read;
    mt_drv_proc_write_func write;
    mt_drv_proc_ioctl_func ioctl;
    mt_void *data;
}mt_proc_entry_t;


typedef struct tag_mt_drvproc
{
    mt_proc_read_func fnRead;
    mt_drv_proc_write_func fnWrite;
    mt_drv_proc_ioctl_func fnIoctl;
}mt_drv_proc_t;

typedef mt_void (* mt_proc_remove_module_func)(char *);
typedef mt_proc_entry_t *(*mt_proc_add_module_fun)(char *, mt_drv_proc_t*, void *);

typedef struct tag_mt_proc_param{
	mt_proc_add_module_fun     addfunc;
	mt_proc_remove_module_func   rmfunc;
}mt_proc_param_t;

mt_s32  mt_drv_proc_set_param(mt_proc_param_t *param);
mt_void mt_drv_proc_clear_param(mt_void);
mt_s32  mt_drv_proc_init(mt_void);
mt_void mt_drv_proc_exit(mt_void);
mt_s32  mt_drv_proc_kinit(mt_void);
mt_void mt_drv_proc_kexit(mt_void);

ssize_t mt_drv_proc_mwrite(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos, mt_proc_ctrl_func fun_ctl);

mt_proc_entry_t* mt_drv_proc_add_module(mt_char *,mt_drv_proc_t*, mt_void *);
mt_void mt_drv_proc_rm_module(mt_char *);


/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __MT_DRV_PROC_H__ */

